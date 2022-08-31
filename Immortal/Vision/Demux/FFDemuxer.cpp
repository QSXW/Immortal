#include "FFDemuxer.h"
#include "Config.h"
#include "Vision/Video/FFCodec.h"
#include "FileSystem/FileSystem.h"
#include "Algorithm/LightVector.h"

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_FFMPEG
class FormatContext : public IObject
{
public:
    FormatContext() :
        handle{}
    {}

    FormatContext(const std::string &filepath) :
        handle{ avformat_alloc_context() },
        streamIndex{}
    {
        ThrowIf(!handle, "Failed to allocated memory for FFCodec::FormatContext");

        memset(streamIndex, -1, sizeof(streamIndex));
        int ret = avformat_open_input(&handle, filepath.c_str(), NULL, NULL);
        if (ret < 0)
        {
            LOG::ERR("FFDemuxer::FormatContext::OpenInput::{}::{}", filepath, ret);
        }
        auto options = GenerateStreamInfo();

        ret = avformat_find_stream_info(handle, options);
        if (ret < 0)
        {
            LOG::ERR("FFDemuxer::FormatContext::FindStreamInfo::{}::{}", filepath, ret);
        }
        av_dump_format(handle, 0, filepath.c_str(), 0);

        streamIndex[AVMEDIA_TYPE_VIDEO]    = FindBestStream(MediaType::Video, streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_AUDIO]    = FindBestStream(MediaType::Audio, streamIndex[AVMEDIA_TYPE_AUDIO], streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_SUBTITLE] = FindBestStream(MediaType::Subtitle, streamIndex[AVMEDIA_TYPE_SUBTITLE], streamIndex[AVMEDIA_TYPE_AUDIO]);

        avformat_seek_file(handle, streamIndex[AVMEDIA_TYPE_VIDEO], 0, 0, 0, 0);
    }

    ~FormatContext()
    {
        if (handle)
        {
            avformat_close_input(&handle);
            avformat_free_context(handle);
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
            codec = handle->oformat ? avcodec_find_encoder(stream->codecpar->codec_id) : avcodec_find_decoder(stream->codecpar->codec_id);

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

    static const char *GetHardwareDecoderNameById(const AVCodecID id)
    {
        switch (id)
        {
        case AV_CODEC_ID_H264:
            return "h264_cuvid";

        case AV_CODEC_ID_HEVC:
            return "hevc_cuvid";

        case AV_CODEC_ID_AV1:
			return "av1_nvdec";

        case AV_CODEC_ID_MJPEG:
			return "mjpeg_nvdec";

        case AV_CODEC_ID_MPEG4:
			return "mpeg4_nvdec";

        case AV_CODEC_ID_MPEG1VIDEO:
			return "mpeg1_nvdec";

        case AV_CODEC_ID_MPEG2VIDEO:
			return "mpeg2_nvdec";

        case AV_CODEC_ID_VC1:
			return "vc1_nvdec";

        case AV_CODEC_ID_WMV3:
			return "wmv3_nvdec";

        case AV_CODEC_ID_VP8:
			return "vp8_nvdec";

        case AV_CODEC_ID_VP9:
			return "vp9_nvdec";

        default:
            return nullptr;
        }
    }

    int OpenStream(FFCodec *ffCodec, MediaType type)
    {
        int ret = 0;
        auto stream = handle->streams[GetIndex(type)];

        auto name = GetHardwareDecoderNameById(stream->codecpar->codec_id);
        auto codec = !name ? avcodec_find_decoder(stream->codecpar->codec_id) : avcodec_find_decoder_by_name(name);
        if (!codec)
        {
            LOG::ERR("There is not suitable codec for {}", handle->url);
        }
        ffCodec->SetCodecContext(codec, stream->codecpar);

        return ret;
    }

    AVStream *GetStream(MediaType type, int index = 0)
    {
        (void)index;
        return handle->streams[(AVMediaType)type];
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

private:
    AVFormatContext *handle;

    int streamIndex[4] = { 0 };

    const AVCodec *codec;
};

FFDemuxer::FFDemuxer() :
    packet{ av_packet_alloc() }
{

}

FFDemuxer::~FFDemuxer()
{
    if (packet)
    {
        av_packet_free(&packet);
    }
}

CodecError FFDemuxer::Open(const std::string &filepath, VideoCodec *codec)
{
    this->filepath = filepath;

    AVInputFormat format{};
    formatContext = new FormatContext{ filepath };
    formatContext->OpenStream(dynamic_cast<FFCodec*>(codec), MediaType::Video);

    auto animator = codec->GetAddress<Animator>();

    AVFormatContext *fc = *formatContext;
    AVStream *stream = formatContext->GetStream(MediaType::Video);
    auto fps = stream->avg_frame_rate;
    animator->FramesPerSecond = fps.den != 0 ? av_q2d(fps) : 24.0f;
    animator->SecondsPerFrame = 1 / animator->FramesPerSecond;
    animator->Duration = fc->duration / AV_TIME_BASE;

    return CodecError::Succeed;
}

CodecError FFDemuxer::Read(CodedFrame *codedFrame)
{
    codedFrame->RefTo(packet);

    do
    {
        av_packet_unref(packet);
        int ret = formatContext->ReadFrame(packet);
        if (ret < 0)
        {
            LOG::DEBUG("Failed to read frame: {}", ret);
            return CodecError::EndOfFile;
        }
    } while (packet->stream_index != formatContext->GetIndex(MediaType::Video));

    AVStream *stream = formatContext->GetStream(MediaType::Video);
    packet->time_base = stream->time_base;

    return CodecError::Succeed;
}
#endif

const std::string &FFDemuxer::GetSource() const
{
    return filepath;
}

}
}

