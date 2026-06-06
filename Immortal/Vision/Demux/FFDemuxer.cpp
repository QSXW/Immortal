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

		int ret = avformat_open_input(&handle, path.c_str(), nullptr, nullptr);
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

        streamIndex[AVMEDIA_TYPE_VIDEO]    = FindBestStream(MediaType::Video,    streamIndex[AVMEDIA_TYPE_VIDEO]);
        streamIndex[AVMEDIA_TYPE_AUDIO]    = FindBestStream(MediaType::Audio,    streamIndex[AVMEDIA_TYPE_AUDIO],    streamIndex[AVMEDIA_TYPE_VIDEO]);
		streamIndex[AVMEDIA_TYPE_DATA]     = FindBestStream(MediaType::Data,     streamIndex[AVMEDIA_TYPE_DATA],     streamIndex[AVMEDIA_TYPE_VIDEO]);
		streamIndex[AVMEDIA_TYPE_SUBTITLE] = FindBestStream(MediaType::Subtitle, streamIndex[AVMEDIA_TYPE_SUBTITLE], streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ? streamIndex[AVMEDIA_TYPE_AUDIO] : streamIndex[AVMEDIA_TYPE_VIDEO]);
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
        animator->Duration        = handle->duration != AV_NOPTS_VALUE ? handle->duration / AV_TIME_BASE : -1;
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

private:
    AVFormatContext *handle;

    int streamIndex[4] = { 0 };

    const AVCodec *codec;
};

FFDemuxer::FFDemuxer() :
    handle{},
    codecs{}
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

CodecError FFDemuxer::Open(const String &_filepath, VideoCodec *codec, VideoCodec *audioCodec, VideoCodec *subtitleCodec)
{
	formatContext.Reset();
    filepath = _filepath;

    AVInputFormat format{};
    formatContext = new FormatContext{ filepath };
    if (!*formatContext)
    {
		return CodecError::ExternalFailed;
    }

    formatContext->OpenStream(codec, MediaType::Video);
    if (audioCodec)
    {
        formatContext->OpenStream(audioCodec, MediaType::Audio);
    }
    if (subtitleCodec)
    {
		formatContext->OpenStream((VideoCodec *)subtitleCodec, MediaType::Subtitle);
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

#define AVERR_STR(ret) av_make_error_string(err, 64, ret)
CodecError FFDemuxer::Open(const String &_filepath, Codec *videoCodec, Codec *audioCodec, Codec *subtitleCodec, const std::initializer_list<MediaType> &&mediaTypes)
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

    if (!videoCodec)
    {
		return CodecError::InvalidArguments;
    }

    codecs[0] = ((FFCodec *)videoCodec)->GetHandle();
	codecs[1] = audioCodec ? ((FFCodec *) audioCodec)->GetHandle() : nullptr;
	codecs[3] = subtitleCodec ? ((FFCodec *) subtitleCodec)->GetHandle() : nullptr;

    auto fmt = handle->oformat;
	for (auto &s : mediaTypes)
    {
		AVCodecContext *codec = codecs[int(s)];
		AVStream *stream  = avformat_new_stream(handle, NULL);
		stream->id        = handle->nb_streams - 1;
		stream->time_base = codec->time_base;
        streams.emplace_back(stream);

		int ret = avcodec_parameters_from_context(stream->codecpar, codec);
        if (ret < 0)
        {
			LOG::ERR("Failed to copy codec parameters from codec");
			return CodecError::ExternalFailed;
        }
		if (fmt->flags & AVFMT_GLOBALHEADER)
		{
			codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
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
			return CodecError::EndOfFile;
        }
        LOG::DEBUG("Failed to read frame: {}", ret);
        return CodecError::EndOfFile;
    }

    if (packet->stream_index != formatContext->GetStreamIndex(MediaType::Video) &&
        packet->stream_index != formatContext->GetStreamIndex(MediaType::Audio) &&
	    packet->stream_index != formatContext->GetStreamIndex(MediaType::Subtitle))
    {
		av_packet_free(&packet);
        return CodecError::ExternalFailed;
    }

    CodedFrame codedFrame;
    auto stream = formatContext->GetStream(packet->stream_index);
  //  if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
  //  {
		//codedFrame = { packet->data, (size_t)packet->size };
  //  }
  //  else
    {
        codedFrame = { packet };
    }

	codedFrame.SetTimestamp(packet->pts);
	codedFrame.SetType((MediaType) stream->codecpar->codec_type);
	packet->time_base = stream->time_base;

	codedFrame.SetRelease([packet](void *data) {
		Async::Execute([packet] {
			//AVPacket *packet = (AVPacket *) (data);
			av_packet_free((AVPacket **)&packet);
		});
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
