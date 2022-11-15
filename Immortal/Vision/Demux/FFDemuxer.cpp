#include "FFDemuxer.h"
#include "Config.h"
#include "Vision/Video/FFCodec.h"
#include "FileSystem/FileSystem.h"
#include "Algorithm/LightVector.h"
#include "Helper/Platform.h"
#include <list>

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

    FormatContext(const std::string &path) :
        handle{ avformat_alloc_context() },
        streamIndex{}
    {
        ThrowIf(!handle, "Failed to allocated memory for FFCodec::FormatContext");

        memset(streamIndex, -1, sizeof(streamIndex));

        std::string filepath = Byte2UTF8(path);

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

    void OpenStream(VideoCodec *ffCodec, MediaType type)
    {
        if (streamIndex[(size_t)type] < 0)
        {
            return;
        }
        auto stream = handle->streams[GetIndex(type)];

		FFDemuxer::Params params = { .stream = stream };
		ffCodec->SetCodecContext(Anonymize(params));

        auto animator = ffCodec->GetAddress<Animator>();

        auto fps = stream->avg_frame_rate;
        animator->FramesPerSecond = fps.den != 0 ? av_q2d(fps) : 24.0f;
        animator->SecondsPerFrame = 1 / animator->FramesPerSecond;
        animator->Duration        = handle->duration / AV_TIME_BASE;
    }

    AVStream *GetStream(MediaType type)
    {
        return handle->streams[streamIndex[(int)type]];
    }

    AVStream *GetStream(int index)
    {
        return handle->streams[index];
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

FFDemuxer::FFDemuxer()
{

}

FFDemuxer::~FFDemuxer()
{
	Destory();
}

void FFDemuxer::Destory()
{
	formatContext.Reset();
}

CodecError FFDemuxer::Open(const std::string &filepath, VideoCodec *codec, VideoCodec *audioCodec)
{
	formatContext.Reset();

    this->filepath = filepath;

    AVInputFormat format{};
    formatContext = new FormatContext{ filepath };
    formatContext->OpenStream(codec, MediaType::Video);

    if (audioCodec)
    {
        formatContext->OpenStream(audioCodec, MediaType::Audio);
    }

    return CodecError::Succeed;
}

CodecError FFDemuxer::Read(CodedFrame *codedFrame)
{
	AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
		return CodecError::OutOfMemory;
    }

    codedFrame->RefTo(packet);
    int ret = formatContext->ReadFrame(packet);
    if (ret < 0)
    {
        av_packet_free(&packet);
        if (ret == AVERROR_EOF)
        {
			return CodecError::EndOfFile;
        }
        LOG::DEBUG("Failed to read frame: {}", ret);
        return CodecError::EndOfFile;
    }

    auto stream = formatContext->GetStream(packet->stream_index);
    codedFrame->Type = (MediaType)stream->codecpar->codec_type;
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

