#include "FFVideoImageCodec.h"

#include "Config.h"
#include "Vision/Common/SamplingFactor.h"
#include "Vision/Video/FFCodec.h"
#include "Shared/Log.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/display.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}
#endif

namespace Immortal
{
namespace Vision
{

FFVideoImageCodec::FFVideoImageCodec() :
    Super{},
    IClass{"FFVideoImageCodec"}
{
}

FFVideoImageCodec::~FFVideoImageCodec()
{
}

#if HAVE_FFMPEG
namespace
{

struct MemoryIO
{
    const uint8_t *data = nullptr;
    size_t size = 0;
    size_t offset = 0;
};

struct FormatContext
{
    ~FormatContext()
    {
        if (handle)
        {
            avformat_close_input(&handle);
        }
        if (io)
        {
            avio_context_free(&io);
        }
    }

    AVFormatContext *handle = nullptr;
    AVIOContext *io = nullptr;
};

struct PacketList
{
    PacketList() = default;

    PacketList(PacketList &&other) noexcept
    {
        packets.swap(other.packets);
    }

    PacketList &operator=(PacketList &&other) noexcept
    {
        if (this != &other)
        {
            Release();
            packets.swap(other.packets);
        }
        return *this;
    }

    PacketList(const PacketList &) = delete;

    PacketList &operator=(const PacketList &) = delete;

    ~PacketList()
    {
        Release();
    }

    void Release()
    {
        for (AVPacket *packet : packets)
        {
            av_packet_free(&packet);
        }
        packets.clear();
    }

    std::vector<AVPacket *> packets;
};

static int ReadMemory(void *opaque, uint8_t *buffer, int bufferSize)
{
    auto *memory = static_cast<MemoryIO *>(opaque);
    if (!memory || !buffer || bufferSize <= 0)
    {
        return AVERROR(EINVAL);
    }
    if (memory->offset >= memory->size)
    {
        return AVERROR_EOF;
    }

    const size_t available = memory->size - memory->offset;
    const size_t bytes = std::min(available, static_cast<size_t>(bufferSize));
    memcpy(buffer, memory->data + memory->offset, bytes);
    memory->offset += bytes;
    return static_cast<int>(bytes);
}

static int64_t SeekMemory(void *opaque, int64_t offset, int whence)
{
    auto *memory = static_cast<MemoryIO *>(opaque);
    if (!memory)
    {
        return AVERROR(EINVAL);
    }
    if ((whence & ~AVSEEK_FORCE) == AVSEEK_SIZE)
    {
        return memory->size <= static_cast<size_t>(std::numeric_limits<int64_t>::max())
            ? static_cast<int64_t>(memory->size)
            : AVERROR(EOVERFLOW);
    }

    int64_t base = 0;
    switch (whence & ~AVSEEK_FORCE)
    {
    case SEEK_SET:
        break;
    case SEEK_CUR:
        base = static_cast<int64_t>(memory->offset);
        break;
    case SEEK_END:
        base = static_cast<int64_t>(memory->size);
        break;
    default:
        return AVERROR(EINVAL);
    }

    if (offset > 0 && base > std::numeric_limits<int64_t>::max() - offset)
    {
        return AVERROR(EOVERFLOW);
    }
    if (offset < -base)
    {
        return AVERROR(EINVAL);
    }

    const int64_t position = base + offset;
    if (position < 0 || static_cast<uint64_t>(position) > memory->size)
    {
        return AVERROR(EINVAL);
    }
    memory->offset = static_cast<size_t>(position);
    return position;
}

static CodecId ToCodecId(AVCodecID id)
{
    switch (id)
    {
    case AV_CODEC_ID_HEVC:
        return CodecId::HEVC;
    case AV_CODEC_ID_AV1:
        return CodecId::AV1;
    case AV_CODEC_ID_VVC:
        return CodecId::VVC;
    case AV_CODEC_ID_H264:
        return CodecId::H264;
    default:
        return CodecId::None;
    }
}

static CodecError OpenInput(const CodedFrame &codedFrame, MemoryIO &memory, FormatContext &format)
{
    if (!codedFrame.GetData() || codedFrame.GetSize() == 0)
    {
        return CodecError::InvalidArguments;
    }
    if (codedFrame.GetSize() > static_cast<size_t>(std::numeric_limits<int64_t>::max()))
    {
        return CodecError::InvalidArguments;
    }

    memory.data = codedFrame.GetData();
    memory.size = codedFrame.GetSize();

    constexpr int kIOBufferSize = 64 * 1024;
    uint8_t *buffer = static_cast<uint8_t *>(av_malloc(kIOBufferSize));
    if (!buffer)
    {
        return CodecError::OutOfMemory;
    }

    format.io = avio_alloc_context(buffer, kIOBufferSize, 0, &memory, ReadMemory, nullptr, SeekMemory);
    if (!format.io)
    {
        av_free(buffer);
        return CodecError::OutOfMemory;
    }
    format.io->seekable = AVIO_SEEKABLE_NORMAL;

    format.handle = avformat_alloc_context();
    if (!format.handle)
    {
        return CodecError::OutOfMemory;
    }
    format.handle->pb = format.io;
    format.handle->flags |= AVFMT_FLAG_CUSTOM_IO;

    int ret = avformat_open_input(&format.handle, nullptr, nullptr, nullptr);
    if (ret < 0)
    {
        return CodecError::CorruptedBitstream;
    }
    ret = avformat_find_stream_info(format.handle, nullptr);
    return ret < 0 ? CodecError::CorruptedBitstream : CodecError::Success;
}

static CodecError OpenDecoder(const AVStream *stream, URef<FFCodec> &decoder)
{
    if (!stream || !stream->codecpar || stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO ||
        stream->codecpar->width <= 0 || stream->codecpar->height <= 0 ||
        ToCodecId(stream->codecpar->codec_id) == CodecId::None)
    {
        return CodecError::UnsupportFormat;
    }

    CodecInfo info{
        .handle = const_cast<AVStream *>(stream),
        .mediaType = MediaType::Video,
        .codecId = ToCodecId(stream->codecpar->codec_id),
        .width = static_cast<uint32_t>(stream->codecpar->width),
        .height = static_cast<uint32_t>(stream->codecpar->height),
        .timeBase = { stream->time_base.num, stream->time_base.den }
    };

    decoder = new FFCodec;
    decoder->SetPreference(DecodingPreference::Software);
    return decoder->OpenDecoder(info);
}

static void ApplyStreamProperties(Picture &picture, const AVStream *stream)
{
    if (!picture || !stream || !stream->codecpar)
    {
        return;
    }

    const AVColorTransferCharacteristic transfer = stream->codecpar->color_trc;
    if (transfer >= AVCOL_TRC_RESERVED0 && transfer <= AVCOL_TRC_ARIB_STD_B67)
    {
        picture.SetColorTransferCharacteristic(static_cast<ColorTransferCharacteristic>(transfer));
    }
}

static CodecError ReceivePicture(FFCodec &decoder, Picture &decoded)
{
    CodecError error = decoder.GetPicture(decoded);
    if (error == CodecError::Success || error == CodecError::EndOfFile)
    {
        return decoded ? CodecError::Success : error;
    }
    return error;
}

static CodecError ReadPackets(
    AVFormatContext *format,
    const std::vector<AVStream *> &streams,
    std::vector<PacketList> &packetLists)
{
    std::unordered_map<int, size_t> streamSlots;
    packetLists.resize(streams.size());

    for (size_t i = 0; i < streams.size(); ++i)
    {
        if (!streams[i])
        {
            return CodecError::CorruptStream;
        }
        if (!streamSlots.emplace(streams[i]->index, i).second)
        {
            return CodecError::CorruptStream;
        }
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        return CodecError::OutOfMemory;
    }

    CodecError result = CodecError::Success;
    while (true)
    {
        const int ret = av_read_frame(format, packet);
        if (ret == AVERROR_EOF)
        {
            break;
        }
        if (ret == AVERROR(EAGAIN) || ret == AVERROR(EINTR))
        {
            continue;
        }
        if (ret < 0)
        {
            result = CodecError::CorruptStream;
            break;
        }

        const auto it = streamSlots.find(packet->stream_index);
        if (it != streamSlots.end())
        {
            AVPacket *owned = av_packet_clone(packet);
            if (!owned)
            {
                result = CodecError::OutOfMemory;
                break;
            }
            owned->time_base = streams[it->second]->time_base;
            packetLists[it->second].packets.emplace_back(owned);
        }
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    if (result != CodecError::Success)
    {
        return result;
    }

    return CodecError::Success;
}

static CodecError DecodePacketList(const AVStream *stream, const PacketList &packetList, Picture &decoded)
{
    if (packetList.packets.empty())
    {
        return CodecError::CorruptStream;
    }

    URef<FFCodec> decoder;
    CodecError error = OpenDecoder(stream, decoder);
    if (error != CodecError::Success)
    {
        return error;
    }

    for (AVPacket *packet : packetList.packets)
    {
        error = decoder->Decode(CodedFrame{ packet });
        if (error == CodecError::Again)
        {
            error = ReceivePicture(*decoder, decoded);
            if (error == CodecError::Success)
            {
                return error;
            }
            error = decoder->Decode(CodedFrame{ packet });
        }
        if (error != CodecError::Success)
        {
            return error;
        }

        error = ReceivePicture(*decoder, decoded);
        if (error == CodecError::Success)
        {
            return error;
        }
        if (error != CodecError::Again)
        {
            return error;
        }
    }

    error = decoder->Decode(CodedFrame{ static_cast<AVPacket *>(nullptr) });
    if (error != CodecError::Success && error != CodecError::EndOfFile)
    {
        return error;
    }
    error = ReceivePicture(*decoder, decoded);
    return error == CodecError::EndOfFile ? CodecError::CorruptStream : error;
}

static bool CopyRectangle(
    Picture &destination,
    const Picture &source,
    int destinationX,
    int destinationY,
    int sourceX,
    int sourceY,
    int width,
    int height)
{
    if (!destination || !source || destination.GetFormat() != source.GetFormat() ||
        destination.GetFormat() == Format::None || destination.GetFormat().IsType(Format::YUYV) ||
        destinationX < 0 || destinationY < 0 || sourceX < 0 || sourceY < 0 ||
        width <= 0 || height <= 0 ||
        destinationX + width > static_cast<int>(destination.GetWidth()) ||
        destinationY + height > static_cast<int>(destination.GetHeight()) ||
        sourceX + width > static_cast<int>(source.GetWidth()) ||
        sourceY + height > static_cast<int>(source.GetHeight()))
    {
        return false;
    }

    const Format format = destination.GetFormat();
    if (format == Format::YUVA420P)
    {
        return false;
    }
    SamplingFactor factors[SamplingFactor::kMaxSublayer]{};
    GetSamplingFactor(format, factors);
    const bool planarYuv = format.IsType(Format::YUV) && !format.IsType(Format::YUYV);
    if (!planarYuv && format.GetTexelSize() == 0)
    {
        return false;
    }
    const int planeCount = planarYuv
        ? (format == Format::YUVA420P ? 4 : (format.IsType(Format::NV) ? 2 : 3))
        : 1;
    const int bytesPerSample = planarYuv
        ? (format.IsType(Format::HightBitDepth) ? 2 : 1)
        : static_cast<int>(format.GetTexelSize());

    for (int plane = 0; plane < planeCount; ++plane)
    {
        if (!destination[plane] || !source[plane])
        {
            return false;
        }

        const int shiftX = factors[plane].x;
        const int shiftY = factors[plane].y;
        const int horizontalAlignment = 1 << shiftX;
        const int verticalAlignment = 1 << shiftY;
        if ((destinationX % horizontalAlignment) != 0 || (sourceX % horizontalAlignment) != 0 ||
            (destinationY % verticalAlignment) != 0 || (sourceY % verticalAlignment) != 0 ||
            ((destinationX + width < static_cast<int>(destination.GetWidth()) ||
              sourceX + width < static_cast<int>(source.GetWidth())) &&
             (width % horizontalAlignment) != 0) ||
            ((destinationY + height < static_cast<int>(destination.GetHeight()) ||
              sourceY + height < static_cast<int>(source.GetHeight())) &&
             (height % verticalAlignment) != 0))
        {
            return false;
        }
        const int unitBytes = bytesPerSample * ((format.IsType(Format::NV) && plane == 1) ? 2 : 1);
        const int planeDestinationX = destinationX >> shiftX;
        const int planeDestinationY = destinationY >> shiftY;
        const int planeSourceX = sourceX >> shiftX;
        const int planeSourceY = sourceY >> shiftY;
        const int planeWidth = (width + ((1 << shiftX) - 1)) >> shiftX;
        const int planeHeight = (height + ((1 << shiftY) - 1)) >> shiftY;
        const size_t rowBytes = static_cast<size_t>(planeWidth) * unitBytes;

        for (int y = 0; y < planeHeight; ++y)
        {
            uint8_t *dst = destination[plane] +
                static_cast<size_t>(planeDestinationY + y) * destination.GetStride(plane) +
                static_cast<size_t>(planeDestinationX) * unitBytes;
            const uint8_t *src = source[plane] +
                static_cast<size_t>(planeSourceY + y) * source.GetStride(plane) +
                static_cast<size_t>(planeSourceX) * unitBytes;
            memcpy(dst, src, rowBytes);
        }
    }
    return true;
}

static void CopyPictureProperties(Picture &destination, const Picture &source)
{
    destination.SetColorSpace(source.GetColorSpace());
    destination.SetColorTransferCharacteristic(source.GetColorTransferCharacteristic());
    destination.SetSampleAspectRatio(source.GetSampleAspectRatio());
    destination.SetTimestamp(source.GetTimestamp());
    destination.SetTimebase(source.GetTimebase());
    if (source.GetFlags() & PictureFlags::FullRange)
    {
        destination.SetFlags(PictureFlags::FullRange);
    }
}

static bool FillPicture(Picture &picture, const uint8_t background[4])
{
    if (!picture || !picture.GetData())
    {
        return false;
    }

    // Chroma-subsampled destinations require an even-sized source in swscale.
    // A 1x1 RGBA source fails for formats such as the Sony HIF's yuv422p10le.
    constexpr uint32_t kColorWidth = 2;
    constexpr uint32_t kColorHeight = 2;
    Picture color{ kColorWidth, kColorHeight, Format::RGBA8, true };
    if (!color || !color.GetData())
    {
        return false;
    }
    for (uint32_t y = 0; y < kColorHeight; ++y)
    {
        uint8_t *row = color.GetData() + static_cast<size_t>(y) * color.GetStride(0);
        for (uint32_t x = 0; x < kColorWidth; ++x)
        {
            memcpy(row + static_cast<size_t>(x) * 4, background, 4);
        }
    }

    Scaler scaler;
    scaler.Init(
        color.GetWidth(),
        color.GetHeight(),
        color.GetFormat(),
        picture.GetWidth(),
        picture.GetHeight(),
        picture.GetFormat(),
        Filter::Nearest);
    return scaler && scaler.Scale(picture, color) == CodecError::Success;
}

static const AVStreamGroup *FindPrimaryTileGrid(const AVFormatContext *format)
{
    const AVStreamGroup *fallback = nullptr;
    for (unsigned int i = 0; i < format->nb_stream_groups; ++i)
    {
        const AVStreamGroup *group = format->stream_groups[i];
        if (!group || group->type != AV_STREAM_GROUP_PARAMS_TILE_GRID || !group->params.tile_grid)
        {
            continue;
        }
        if (!fallback)
        {
            fallback = group;
        }
        if (group->disposition & AV_DISPOSITION_DEFAULT)
        {
            return group;
        }
    }
    return fallback;
}

static const int32_t *FindDisplayMatrix(const AVStreamGroupTileGrid *grid)
{
    if (!grid)
    {
        return nullptr;
    }
    const AVPacketSideData *sideData = av_packet_side_data_get(
        grid->coded_side_data,
        grid->nb_coded_side_data,
        AV_PKT_DATA_DISPLAYMATRIX);
    return sideData && sideData->size >= sizeof(int32_t) * 9
        ? reinterpret_cast<const int32_t *>(sideData->data)
        : nullptr;
}

static void SetDisplayOrientation(Picture &picture, const int32_t *matrix)
{
    if (!matrix || !picture)
    {
        return;
    }

    double m00 = matrix[0];
    double m01 = matrix[1];
    double m10 = matrix[3];
    double m11 = matrix[4];
    const double scaleX = std::hypot(m00, m10);
    const double scaleY = std::hypot(m01, m11);
    if (scaleX <= 0.0 || scaleY <= 0.0)
    {
        return;
    }

    m00 /= scaleX;
    m10 /= scaleX;
    m01 /= scaleY;
    m11 /= scaleY;
    const double determinant = m00 * m11 - m01 * m10;
    if (std::fabs(determinant) < 0.5)
    {
        return;
    }

    const double transform[4] = { m00, m01, m10, m11 };
    struct Candidate
    {
        int hflip;
        int rotation;
        double transform[4];
    };
    static constexpr Candidate candidates[] = {
        { 0,   0, {  1,  0,  0,  1 } },
        { 0,  90, {  0, -1,  1,  0 } },
        { 0, 180, { -1,  0,  0, -1 } },
        { 0, 270, {  0,  1, -1,  0 } },
        { 1,   0, { -1,  0,  0,  1 } },
        { 1,  90, {  0,  1,  1,  0 } },
        { 1, 180, {  1,  0,  0, -1 } },
        { 1, 270, {  0, -1, -1,  0 } }
    };

    const Candidate *best = nullptr;
    double bestError = std::numeric_limits<double>::max();
    for (const Candidate &candidate : candidates)
    {
        double error = 0.0;
        for (size_t i = 0; i < 4; ++i)
        {
            const double delta = transform[i] - candidate.transform[i];
            error += delta * delta;
        }
        if (error < bestError)
        {
            best = &candidate;
            bestError = error;
        }
    }
    if (!best || bestError > 0.1)
    {
        LOG_WARNING("Unsupported still-image display matrix");
        return;
    }

    auto *orientation = picture.AllocateProperty<DisplayOrientation>();
    orientation->anticlockwiseRotation = best->rotation == 0 ? 0 : -best->rotation;
    orientation->hflip = best->hflip;
}

static CodecError ComposeTileGrid(
    const AVStreamGroup *group,
    const std::vector<PacketList> &packetLists,
    Picture &output)
{
    const AVStreamGroupTileGrid *grid = group ? group->params.tile_grid : nullptr;
    if (!grid || grid->nb_tiles == 0 || group->nb_streams == 0 || !group->streams ||
        grid->coded_width <= 0 || grid->coded_height <= 0 ||
        grid->width <= 0 || grid->height <= 0 ||
        grid->horizontal_offset < 0 || grid->vertical_offset < 0 ||
        grid->horizontal_offset >= grid->coded_width || grid->vertical_offset >= grid->coded_height ||
        grid->width > grid->coded_width - grid->horizontal_offset ||
        grid->height > grid->coded_height - grid->vertical_offset ||
        av_image_check_size(grid->width, grid->height, 0, nullptr) < 0 ||
        packetLists.size() != group->nb_streams)
    {
        return CodecError::CorruptStream;
    }

    if (!grid->offsets || grid->offsets[0].idx >= group->nb_streams)
    {
        return CodecError::CorruptStream;
    }

    const unsigned int firstStreamIndex = grid->offsets[0].idx;
    Picture firstTile;
    CodecError error = DecodePacketList(
        group->streams[firstStreamIndex],
        packetLists[firstStreamIndex],
        firstTile);
    if (error != CodecError::Success || !firstTile)
    {
        return error;
    }
    ApplyStreamProperties(firstTile, group->streams[firstStreamIndex]);
    if (firstTile.GetFormat() == Format::None || firstTile.GetFormat() == Format::YUVA420P ||
        firstTile.GetFormat().IsType(Format::YUYV))
    {
        return CodecError::UnsupportFormat;
    }

    output = Picture{
        static_cast<uint32_t>(grid->width),
        static_cast<uint32_t>(grid->height),
        firstTile.GetFormat(),
        true
    };
    if (!output || !output.GetData())
    {
        return CodecError::OutOfMemory;
    }
    if (!FillPicture(output, grid->background))
    {
        LOG_ERROR(
            "FFVideoImageCodec failed to initialize the {}x{} tile-grid background for pixel format {}",
            grid->width,
            grid->height,
            firstTile.GetFormat().GetString());
        return CodecError::ExternalFailed;
    }
    CopyPictureProperties(output, firstTile);
    SetDisplayOrientation(output, FindDisplayMatrix(grid));

    for (unsigned int i = 0; i < grid->nb_tiles; ++i)
    {
        const auto &offset = grid->offsets[i];
        if (offset.idx >= packetLists.size() || !group->streams[offset.idx] ||
            offset.horizontal < 0 || offset.vertical < 0 ||
            offset.horizontal >= grid->coded_width || offset.vertical >= grid->coded_height)
        {
            return CodecError::CorruptStream;
        }

        Picture tile = i == 0 ? firstTile : Picture{};
        if (!tile)
        {
            error = DecodePacketList(group->streams[offset.idx], packetLists[offset.idx], tile);
            if (error != CodecError::Success)
            {
                return error;
            }
        }
        if (tile.GetFormat() != output.GetFormat())
        {
            return CodecError::UnsupportFormat;
        }

        const int64_t tileLeft = offset.horizontal;
        const int64_t tileTop = offset.vertical;
        const int64_t tileRight = tileLeft + tile.GetWidth();
        const int64_t tileBottom = tileTop + tile.GetHeight();
        const int64_t imageLeft = grid->horizontal_offset;
        const int64_t imageTop = grid->vertical_offset;
        const int64_t imageRight = imageLeft + grid->width;
        const int64_t imageBottom = imageTop + grid->height;
        const int64_t copyLeft = std::max(tileLeft, imageLeft);
        const int64_t copyTop = std::max(tileTop, imageTop);
        const int64_t copyRight = std::min(tileRight, imageRight);
        const int64_t copyBottom = std::min(tileBottom, imageBottom);
        if (copyLeft >= copyRight || copyTop >= copyBottom)
        {
            continue;
        }

        const int sourceX = static_cast<int>(copyLeft - tileLeft);
        const int sourceY = static_cast<int>(copyTop - tileTop);
        const int destinationX = static_cast<int>(copyLeft - imageLeft);
        const int destinationY = static_cast<int>(copyTop - imageTop);
        const int copyWidth = static_cast<int>(copyRight - copyLeft);
        const int copyHeight = static_cast<int>(copyBottom - copyTop);
        if (!CopyRectangle(
            output,
            tile,
            destinationX,
            destinationY,
            sourceX,
            sourceY,
            copyWidth,
            copyHeight))
        {
            return CodecError::CorruptStream;
        }
        if (i == 0)
        {
            firstTile = {};
        }
    }

    return CodecError::Success;
}

static AVStream *FindPrimaryImageStream(AVFormatContext *format)
{
    AVStream *fallback = nullptr;
    for (unsigned int i = 0; i < format->nb_streams; ++i)
    {
        AVStream *stream = format->streams[i];
        if (!stream || !stream->codecpar || stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO ||
            (stream->disposition & AV_DISPOSITION_DEPENDENT))
        {
            continue;
        }
        if (!fallback)
        {
            fallback = stream;
        }
        if (stream->disposition & AV_DISPOSITION_DEFAULT)
        {
            return stream;
        }
    }
    return fallback;
}

static const int32_t *FindDisplayMatrix(const AVStream *stream)
{
    if (!stream || !stream->codecpar)
    {
        return nullptr;
    }
    const AVPacketSideData *sideData = av_packet_side_data_get(
        stream->codecpar->coded_side_data,
        stream->codecpar->nb_coded_side_data,
        AV_PKT_DATA_DISPLAYMATRIX);
    return sideData && sideData->size >= sizeof(int32_t) * 9
        ? reinterpret_cast<const int32_t *>(sideData->data)
        : nullptr;
}

}
#endif

CodecError FFVideoImageCodec::Decode(const CodedFrame &codedFrame)
{
#if HAVE_FFMPEG
    picture = {};
    MemoryIO memory{};
    FormatContext format{};
    CodecError error = OpenInput(codedFrame, memory, format);
    if (error != CodecError::Success)
    {
        return error;
    }

    if (const AVStreamGroup *group = FindPrimaryTileGrid(format.handle))
    {
        if (group->nb_streams == 0 || !group->streams)
        {
            return CodecError::CorruptStream;
        }
        std::vector<AVStream *> streams(group->streams, group->streams + group->nb_streams);
        std::vector<PacketList> packetLists;
        error = ReadPackets(format.handle, streams, packetLists);
        return error == CodecError::Success ? ComposeTileGrid(group, packetLists, picture) : error;
    }

    AVStream *stream = FindPrimaryImageStream(format.handle);
    if (!stream)
    {
        return CodecError::NotFound;
    }

    std::vector<PacketList> packetLists;
    error = ReadPackets(format.handle, { stream }, packetLists);
    if (error != CodecError::Success || packetLists.empty())
    {
        return error;
    }
    error = DecodePacketList(stream, packetLists[0], picture);
    if (error == CodecError::Success)
    {
        ApplyStreamProperties(picture, stream);
        SetDisplayOrientation(picture, FindDisplayMatrix(stream));
    }
    return error;
#else
    (void)codedFrame;
    return CodecError::NotImplement;
#endif
}

CodecError FFVideoImageCodec::Encode(const Picture &source, CodedFrame &codedFrame)
{
    (void)source;
    (void)codedFrame;
    return CodecError::NotImplement;
}

}
}
