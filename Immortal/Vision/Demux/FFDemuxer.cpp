#include "FFDemuxer.h"
#include "Config.h"
#include "Vision/Video/FFCodec.h"
#include "FileSystem/FileSystem.h"
#include "Algorithm/LightVector.h"
#include "Helper/Platform.h"

#include <list>
#include <cmath>

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/display.h>
}
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_FFMPEG

#define AVERR_STR(ret) av_make_error_string(err, 64, ret)

static CodecId CAST(AVCodecID codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_H264:
        return CodecId::H264;

    case AV_CODEC_ID_HEVC:
        return CodecId::HEVC;

    case AV_CODEC_ID_VVC:
        return CodecId::VVC;

    case AV_CODEC_ID_VP9:
        return CodecId::VP9;

    case AV_CODEC_ID_AV1:
        return CodecId::AV1;

    case AV_CODEC_ID_AAC:
        return CodecId::AAC;

    case AV_CODEC_ID_FLAC:
        return CodecId::FLAC;

    case AV_CODEC_ID_TIFF:
        return CodecId::TIFF;

    case AV_CODEC_ID_PNG:
        return CodecId::PNG;

    case AV_CODEC_ID_WEBP:
        return CodecId::WEBP;

    case AV_CODEC_ID_JPEGXL:
        return CodecId::JPEGXL;

    case AV_CODEC_ID_MPEG4:
        return CodecId::MPEG4;

    default:
		return CodecId::None;
    }
}

Format CAST(AVPixelFormat v);
AVPixelFormat CAST(Format format);

class FormatContext : public IObject
{
public:
    FormatContext() :
        handle{}
    {}

    FormatContext(const String &path) :
        handle{ avformat_alloc_context() },
	    streamIndex{-1,-1,-1,-1}
    {
		char err[64];
        ThrowIf(!handle, "Failed to allocated memory for FFCodec::FormatContext");

        const AVInputFormat *inputFormat = nullptr;
        if (path.size() > 6 && !memcmp(path.c_str(), "video=", 6))
        {
			avdevice_register_all();
			inputFormat = av_find_input_format("dshow");
        }
		int ret = avformat_open_input(&handle, path.c_str(), inputFormat, nullptr);
        if (ret < 0)
        {
			LOG::ERR("AVFormatContext: failed to open {} - {}", path, AVERR_STR(ret));
            return;
        }

        auto options = GenerateStreamInfo();
        ret = avformat_find_stream_info(handle, options);
        if (ret < 0)
        {
			LOG::ERR("AVFormatContext: failed to find stream info {} - {}", path, AVERR_STR(ret));
        }
        av_dump_format(handle, 0, path.c_str(), 0);

        streamIndex[AVMEDIA_TYPE_VIDEO]    = FindBestStream(MediaType::Video,    streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_AUDIO]    = FindBestStream(MediaType::Audio,    streamIndex[AVMEDIA_TYPE_AUDIO],    streamIndex[AVMEDIA_TYPE_VIDEO]);
		streamIndex[AVMEDIA_TYPE_DATA]     = FindBestStream(MediaType::Data,     streamIndex[AVMEDIA_TYPE_DATA],     streamIndex[AVMEDIA_TYPE_VIDEO]);
		streamIndex[AVMEDIA_TYPE_SUBTITLE] = FindBestStream(MediaType::Subtitle, streamIndex[AVMEDIA_TYPE_SUBTITLE], streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ? streamIndex[AVMEDIA_TYPE_AUDIO] : streamIndex[AVMEDIA_TYPE_VIDEO]);

        SetAnimator();
    }

    ~FormatContext()
    {
        if (handle)
        {
            avformat_close_input(&handle);
            avformat_free_context(handle);
        }
    }

    void SetAnimator()
    {
		animators.resize(handle->nb_streams);
        for (size_t i = 0; i < animators.size(); i++)
        {
			auto &stream  = handle->streams[i];
			auto animator = &animators[i];
			auto fps = stream->avg_frame_rate;
			animator->FramesPerSecond = fps.den != 0 ? av_q2d(fps) : 24.0f;
			animator->SecondsPerFrame = 1 / animator->FramesPerSecond;
			animator->Duration = handle->duration != AV_NOPTS_VALUE ? handle->duration / AV_TIME_BASE : -1;

            animator->Framerate = { fps.num, fps.den };
            if (fps.den == 0)
            {
				animator->Framerate = {24, 1};
            }

            animator->TimebaseRational = {stream->time_base.num, stream->time_base.den};
			if (stream->duration != AV_NOPTS_VALUE && stream->time_base.den != 0)
			{
				int64_t num = (int64_t)stream->duration * (int64_t)stream->time_base.num;
				int den = stream->time_base.den;
				animator->DurationRational = Rational((int)num, den);
			}
			else if (handle->duration != AV_NOPTS_VALUE)
			{
				animator->DurationRational = Rational(handle->duration, AV_TIME_BASE);
			}
			else
			{
				// Default to zero duration
				animator->DurationRational = Rational(0, 1);
			}
        }
    }

    /** Find the best stream with index
     */
    int FindBestStream(MediaType type, int request, int related = -1)
    {
        return av_find_best_stream(handle, (AVMediaType)type, request, related, NULL, 0);
    }

    AVDictionary **GenerateStreamInfo()
    {
        AVDictionary **options = nullptr;
        if (!handle->nb_streams)
        {
            return options;
        }

        options = (AVDictionary **)av_calloc(handle->nb_streams, sizeof(AVDictionary*));
        for (size_t i = 0; i < handle->nb_streams; i++)
        {
            auto stream = handle->streams[i];
            int flags = handle->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
            //codec = handle->oformat ? avcodec_find_encoder(stream->codecpar->codec_id) : avcodec_find_decoder(stream->codecpar->codec_id);

            switch (stream->codecpar->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
                flags |= AV_OPT_FLAG_VIDEO_PARAM;
                break;

            case AVMEDIA_TYPE_AUDIO:
                flags |= AV_OPT_FLAG_AUDIO_PARAM;
                break;

            case AVMEDIA_TYPE_SUBTITLE:
                flags |= AV_OPT_FLAG_SUBTITLE_PARAM;
                break;
            }
        }

        return options;
    }

    AVStream *GetStream(MediaType type)
    {
        return handle->streams[streamIndex[(int)type]];
    }

    AVStream *GetStream(int index)
    {
        return handle->streams[index];
    }

    int GetStreamIndex(MediaType type)
    {
        return streamIndex[(int)type];
    }

    operator AVFormatContext *() const
    {
        return handle;
    }

    int ReadFrame(AVPacket *packet)
    {
        return av_read_frame(handle, packet);
    }

    int GetIndex(MediaType type) const
    {
        return streamIndex[(AVMediaType)type];
    }

    int64_t GetDuration() const
    {
        return handle->duration;
    }

    operator bool() const
    {
		return !!handle;
    }

    void EnumerateTracks(MediaType mediaType, std::vector<TrackInfo> &tracks)
    {
		tracks.resize(0);
        for (int i = 0; i < handle->nb_streams; i++)
        {
			auto &stream = handle->streams[i];
            if (stream->codecpar->codec_type == (AVMediaType)mediaType)
			{
				AVDictionaryEntry *tag = av_dict_get(stream->metadata, "title", nullptr, 0);
				tracks.emplace_back(TrackInfo{
				    .name        = tag ? tag->value : std::string("Track#") + std::to_string(i) + std::string(": ") + avcodec_get_name(stream->codecpar->codec_id),
					.streamIndex = i
                    });
            }
        }
    }

public:
    AVFormatContext *handle;

    int streamIndex[4] = { 0 };

    std::vector<Animator> animators;
};

FFDemuxer::FFDemuxer() :
    handle{}
{

}

FFDemuxer::~FFDemuxer()
{
    if (handle)
    {
		Close();
    }
	Destroy();
}

void FFDemuxer::Destroy()
{
	formatContext.Reset();
}

CodecError FFDemuxer::Open(const String &_filepath)
{
	formatContext.Reset();
    filepath = _filepath;

    AVInputFormat format{};
    formatContext = new FormatContext{ filepath };
    if (!*formatContext)
    {
		return CodecError::ExternalFailed;
    }

    return CodecError::Success;
}

static AVCodecID CAST(const CodecId id)
{
    switch (id)
    {
    case CodecId::H264:
    case CodecId::H264_NVENC:
    case CodecId::H264_QSV:
		return AV_CODEC_ID_H264;

    case CodecId::HEVC:
    case CodecId::HEVC_D3D12ENCODE:
    case CodecId::HEVC_NVENC:
    case CodecId::HEVC_QSV:
		return AV_CODEC_ID_HEVC;

    case CodecId::AV1:
	case CodecId::AV1_NVENC:
	case CodecId::AV1_QSV:
		return AV_CODEC_ID_AV1;

    case CodecId::AAC:
		return AV_CODEC_ID_AAC;

    default:
        return AV_CODEC_ID_NONE;
    }
}

CodecError FFDemuxer::Open(const String &_filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec)
{
	int ret = 0;
	char err[64] = {};

    const char *filepath = _filepath.c_str();
	avformat_alloc_output_context2(&handle, nullptr, nullptr, filepath);
    if (!handle)
    {
		LOG::ERR("Failed to alloc output context2 for format context");
		return CodecError::ExternalFailed;
    }

    if (!pCodec)
    {
		return CodecError::InvalidArguments;
    }

    auto fmt = handle->oformat;
	for (uint32_t i = 0; i < numCodec; i++)
    {
		AVCodecContext *codec = ((FFCodec *)pCodec[i])->GetHandle();
		codecs[i] = codec;
		AVStream *stream  = avformat_new_stream(handle, NULL);
		stream->id        = handle->nb_streams - 1;
		stream->time_base = codec->time_base;

		int ret = avcodec_parameters_from_context(stream->codecpar, codec);
        if (ret < 0)
        {
			LOG::ERR("Failed to copy codec parameters from codec");
			return CodecError::ExternalFailed;
        }

		if (pCodec[i]->GetMediaType() == MediaType::Video)
		{
			stream->avg_frame_rate = codec->framerate;

            auto &displayOrientation = encodeInfos[i].displayOrientation;
            if (displayOrientation.hflip != 0 || displayOrientation.anticlockwiseRotation != 0)
            {
				auto &codecpar = stream->codecpar;
				AVPacketSideData *sideData = av_packet_side_data_new(&codecpar->coded_side_data,
                    &codecpar->nb_coded_side_data, AV_PKT_DATA_DISPLAYMATRIX, sizeof(int32_t) * 9, 0);

                int32_t *displayMatrix = (int32_t *)sideData->data;
                if (displayMatrix)
                {
                    if (displayOrientation.anticlockwiseRotation != 0)
                    {
                        av_display_rotation_set(displayMatrix, displayOrientation.anticlockwiseRotation);
                    }
                    if (displayOrientation.hflip)
                    {
                        av_display_matrix_flip(displayMatrix, 1, 0);
                    }
                }
            }
		}

		if (fmt->flags & AVFMT_GLOBALHEADER)
		{
			codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

        streams.emplace_back(stream);
    }

    if (!(fmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&handle->pb, filepath, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			LOG::ERR("Could not open '{}': {}", filepath, AVERR_STR(ret));
			return CodecError::ExternalFailed;
		}
	}

    ret = avformat_write_header(handle, NULL);
    if (ret < 0)
	{
		if (handle->pb)
		{
			avio_closep(&handle->pb);
		}
		avformat_free_context(handle);
		handle = nullptr;

		LOG::ERR("Error occurred when opening output file for writing header: {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

    return CodecError::Success;
}

void FFDemuxer::Close()
{
	int ret = {};
	char err[64] = {};

    if (!handle)
    {
		return;
    }

	if (av_write_trailer(handle) < 0)
	{
		LOG::ERR("Error writing trailer: {}", AVERR_STR(ret));
		return;
	}

    if (!(handle->oformat && (handle->oformat->flags & AVFMT_NOFILE)))
    {
		avio_closep(&handle->pb);
    }

    avformat_free_context(handle);
	handle = {};
}

CodecError FFDemuxer::Read(CodedFrame *pCodedFrame)
{
	char err[64];
	AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
		return CodecError::OutOfMemory;
    }

    int ret = formatContext->ReadFrame(packet);
    if (ret < 0)
    {
        av_packet_free(&packet);
        if (ret == AVERROR_EOF)
        {
			*pCodedFrame = {(AVPacket *)nullptr};
			return CodecError::EndOfFile;
        }
		LOG::DEBUG("Failed to read frame: {}", AVERR_STR(ret));
        return CodecError::EndOfFile;
    }

  //  if (!(packet->flags & AV_PKT_FLAG_KEY))
  //  {
		//av_packet_free(&packet);
		//return CodecError::Again;
  //  }

    if (packet->stream_index != formatContext->GetStreamIndex(MediaType::Video) &&
        packet->stream_index != formatContext->GetStreamIndex(MediaType::Audio) &&
	    packet->stream_index != formatContext->GetStreamIndex(MediaType::Subtitle))
    {
		av_packet_free(&packet);
        return CodecError::ExternalFailed;
    }

    auto stream = formatContext->GetStream(packet->stream_index);
	CodedFrame codedFrame{ packet->data, (size_t)packet->size };
	codedFrame.SetAnonymous(packet);

	codedFrame.SetTimestamp(packet->pts);
	codedFrame.SetType((MediaType) stream->codecpar->codec_type);
	packet->time_base = stream->time_base;

	codedFrame.SetRelease([packet](void *data) {
		//AVPacket *packet = (AVPacket *) (data);
		av_packet_free((AVPacket **)&packet);
	});

	*pCodedFrame = codedFrame;

    return CodecError::Success;
}

CodecError FFDemuxer::Write(const CodedFrame &codedFrame, int stream)
{
	auto packet = codedFrame.InterpretAs<AVPacket>();
    packet->stream_index = stream;

    av_packet_rescale_ts(packet, codecs[stream]->time_base, streams[stream]->time_base);
	int ret = av_interleaved_write_frame(handle, packet);
    if (ret < 0)
    {
		char err[64] = {};
		LOG::ERR("Error while writing output packet : {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
    }

    return CodecError::Success;
}

CodecError FFDemuxer::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	int streamIndex = formatContext->GetStreamIndex(type);
	auto stream = formatContext->GetStream(streamIndex);
	if (avformat_seek_file(*formatContext, streamIndex, min, pts + stream->start_time, max, AVSEEK_FLAG_BACKWARD) < 0)
	{
		return CodecError::ExternalFailed;
	}

	return CodecError::Success;
}

CodecError FFDemuxer::GetStreamInfo(MediaType type, CodecInfo &streamInfo)
{
	auto index = formatContext->GetStreamIndex(type);
    if (index < 0)
    {
		return CodecError::NotFound;
    }

	AVStream *stream = formatContext->GetStream(index);
    auto &codecpar = stream->codecpar;
	streamInfo = CodecInfo{
        .handle    = stream,
		.mediaType = type,
		.codecId   = CAST(codecpar->codec_id),
        .width     = uint32_t(codecpar->width),
		.height    = uint32_t(codecpar->height),
        .format    = CAST(AVPixelFormat(codecpar->format)),
		.bitRate   = int(codecpar->bit_rate),
		.gopSize   = 0,
		.framerate = { codecpar->framerate.num, codecpar->framerate.den },
		.timeBase  = { stream->time_base.num, stream->time_base.den },
		.displayOrientation = {}
	};

    if (codecpar->codec_id == AV_CODEC_ID_NONE)
    {
		return CodecError::InvalidArguments;
    }

    return CodecError::Success;
}

void FFDemuxer::EnumerateTracks(MediaType mediaType, std::vector<TrackInfo> &tracks)
{
	formatContext->EnumerateTracks(mediaType, tracks);
}

CodecError FFDemuxer::SwitchTrack(MediaType mediaType, int index)
{
    if (mediaType > MediaType::Subtitle)
    {
		return CodecError::InvalidArguments;
    }

	formatContext->streamIndex[int(mediaType)] = index;

    return CodecError::Success;
}

Animator &FFDemuxer::GetAnimator(MediaType mediaType)
{
	return formatContext->animators[formatContext->streamIndex[int(mediaType)]];
}

#endif

const String &FFDemuxer::GetSource() const
{
    return filepath;
}

}
}
