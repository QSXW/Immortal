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

    FormatContext(const String &path) :
        handle{ avformat_alloc_context() },
        streamIndex{}
    {
        ThrowIf(!handle, "Failed to allocated memory for FFCodec::FormatContext");

        memset(streamIndex, -1, sizeof(streamIndex));

		int ret = avformat_open_input(&handle, path.c_str(), NULL, NULL);
        if (ret < 0)
        {
            LOG::ERR("FFDemuxer::FormatContext::OpenInput::{}::{}", path, ret);
            return;
        }
        auto options = GenerateStreamInfo();

        ret = avformat_find_stream_info(handle, options);
        if (ret < 0)
        {
            LOG::ERR("FFDemuxer::FormatContext::FindStreamInfo::{}::{}", path, ret);
        }
        av_dump_format(handle, 0, path.c_str(), 0);

        streamIndex[AVMEDIA_TYPE_VIDEO]    = FindBestStream(MediaType::Video, streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_AUDIO]    = FindBestStream(MediaType::Audio, streamIndex[AVMEDIA_TYPE_AUDIO], streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_SUBTITLE] = FindBestStream(MediaType::Subtitle, streamIndex[AVMEDIA_TYPE_SUBTITLE], streamIndex[AVMEDIA_TYPE_AUDIO]);
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
		ffCodec->SetCodecContext((void *)&params);

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

CodecError FFDemuxer::Open(const String &_filepath, VideoCodec *codec, VideoCodec *audioCodec)
{
	formatContext.Reset();

    filepath = _filepath;

    AVInputFormat format{};
    formatContext = new FormatContext{ filepath };
    formatContext->OpenStream(codec, MediaType::Video);

    if (audioCodec)
    {
        formatContext->OpenStream(audioCodec, MediaType::Audio);
    }

    return CodecError::Success;
}

CodecError FFDemuxer::Read(CodedFrame *pCodedFrame)
{
	AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
		return CodecError::OutOfMemory;
    }

    CodedFrame codedFrame = { packet };
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

    if (packet->stream_index != formatContext->GetStreamIndex(MediaType::Video) &&
        packet->stream_index != formatContext->GetStreamIndex(MediaType::Audio))
    {
        return CodecError::ExternalFailed;
    }

    auto stream = formatContext->GetStream(packet->stream_index);
    codedFrame.SetType((MediaType)stream->codecpar->codec_type);
    packet->time_base = stream->time_base;

    codedFrame.SetRelease([] (void *data) {
        Async::Execute([=] {
			AVPacket *packet = (AVPacket *)(data);
			av_packet_unref(packet);
			av_packet_free(&packet);
            });
        });
    *pCodedFrame = codedFrame;

    return CodecError::Success;
}

CodecError FFDemuxer::Seek(MediaType type, double seconds, int64_t min, int64_t max)
{
    int streamIndex = formatContext->GetStreamIndex(type);
    auto stream = formatContext->GetStream(streamIndex);
	int64_t timestamp = av_rescale_q(seconds * AV_TIME_BASE, {1, AV_TIME_BASE}, stream->time_base) + stream->start_time;

    if (avformat_seek_file(*formatContext, formatContext->GetStreamIndex(MediaType::Video), min, timestamp, max, 0) < 0)
    {
        return CodecError::ExternalFailed;
    }

    return CodecError::Success;
}
#endif

const String &FFDemuxer::GetSource() const
{
    return filepath;
}

}
}
