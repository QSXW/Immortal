#include "FFFormat.h"
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

static AVDictionary **GenerateStreamInfo(AVFormatContext *handle)
{
	AVDictionary **options = nullptr;
	if (!handle->nb_streams)
	{
		return options;
	}

	options = (AVDictionary **)av_calloc(handle->nb_streams, sizeof(AVDictionary *));
	for (size_t i = 0; i < handle->nb_streams; i++)
	{
		auto stream = handle->streams[i];
		int flags = handle->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
		// codec = handle->oformat ? avcodec_find_encoder(stream->codecpar->codec_id) : avcodec_find_decoder(stream->codecpar->codec_id);

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

FFFormat::FFFormat() :
    handle{},
    streamIndex{-1,-1,-1,-1},
    animators{}
{

}

FFFormat::~FFFormat()
{
	if (handle)
	{
		Close();
	}
}

void FFFormat::SetAnimator()
{
	animators.resize(handle->nb_streams);
	for (size_t i = 0; i < animators.size(); i++)
	{
		auto &stream = handle->streams[i];
		auto animator = &animators[i];
		auto fps = stream->r_frame_rate;
		animator->FramesPerSecond = fps.den != 0 ? av_q2d(fps) : 24.0f;
		animator->SecondsPerFrame = 1 / animator->FramesPerSecond;
		animator->Duration = handle->duration != AV_NOPTS_VALUE ? handle->duration / AV_TIME_BASE : -1;

		animator->Framerate = {fps.num, fps.den};
		if (fps.den == 0)
		{
			animator->Framerate = {24, 1};
		}

		animator->TimebaseRational = {stream->time_base.num, stream->time_base.den};

		if (stream->duration != AV_NOPTS_VALUE && stream->time_base.den != 0)
		{
			int64_t num = (int64_t) stream->duration * (int64_t) stream->time_base.num;
			int den = stream->time_base.den;
			animator->DurationRational = Rational((int) num, den);
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

CodecError FFFormat::Open(const String &_filepath)
{
    filepath = _filepath;

    handle = avformat_alloc_context();
    if (!handle)
    {
		CLOG_ERROR("Failed to allocated memory for AVFormatContext");
		return CodecError::OutOfMemory;
    }

    char err[64];
	const AVInputFormat *inputFormat = nullptr;
	if (filepath.size() > 6 && !memcmp(filepath.c_str(), "video=", 6))
	{
		avdevice_register_all();
		inputFormat = av_find_input_format("dshow");
	}
	int ret = avformat_open_input(&handle, filepath.c_str(), inputFormat, nullptr);
	if (ret < 0)
	{
		CLOG_ERROR("Failed to open {} - {}", filepath, AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

	auto options = GenerateStreamInfo(handle);
	ret = avformat_find_stream_info(handle, options);
	if (ret < 0)
	{
		CLOG_ERROR("Failed to find stream info {} - {}", filepath, AVERR_STR(ret));
	}
	av_dump_format(handle, 0, filepath.c_str(), 0);

	streamIndex[AVMEDIA_TYPE_VIDEO]    = av_find_best_stream(handle, AVMEDIA_TYPE_VIDEO,    streamIndex[AVMEDIA_TYPE_VIDEO],    -1,                              nullptr, 0);
	streamIndex[AVMEDIA_TYPE_AUDIO]    = av_find_best_stream(handle, AVMEDIA_TYPE_AUDIO,    streamIndex[AVMEDIA_TYPE_AUDIO],    streamIndex[AVMEDIA_TYPE_VIDEO], nullptr, 0);
	streamIndex[AVMEDIA_TYPE_DATA]     = av_find_best_stream(handle, AVMEDIA_TYPE_DATA,     streamIndex[AVMEDIA_TYPE_DATA],     streamIndex[AVMEDIA_TYPE_VIDEO], nullptr, 0);
	streamIndex[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(handle, AVMEDIA_TYPE_SUBTITLE, streamIndex[AVMEDIA_TYPE_SUBTITLE], streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ? streamIndex[AVMEDIA_TYPE_AUDIO] : streamIndex[AVMEDIA_TYPE_VIDEO], nullptr, 0);

	SetAnimator();

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

CodecError FFFormat::Open(const String &_filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec)
{
	int ret = 0;
	char err[64] = {};

    const char *filepath = _filepath.c_str();
	avformat_alloc_output_context2(&handle, nullptr, nullptr, filepath);
    if (!handle)
    {
		CLOG_ERROR("Failed to alloc output context2 for format context");
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
			CLOG_ERROR("Failed to copy codec parameters from codec");
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
			CLOG_ERROR("Could not open '{}': {}", filepath, AVERR_STR(ret));
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

		CLOG_ERROR("Error occurred when opening output file for writing header: {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

    return CodecError::Success;
}

void FFFormat::Close()
{
	int ret = {};
	char err[64] = {};

    if (!handle)
    {
		return;
    }

	if (handle->oformat)
	{
		if (av_write_trailer(handle) < 0)
		{
			CLOG_ERROR("Error writing trailer: {}", AVERR_STR(ret));
			return;
		}

		if (!(handle->oformat && (handle->oformat->flags & AVFMT_NOFILE)))
		{
			avio_closep(&handle->pb);
		}

		avformat_free_context(handle);
		handle = {};
	}
	else
	{
		avformat_close_input(&handle);
	}
}

CodecError FFFormat::Read(CodedFrame *pCodedFrame)
{
	char err[64];
	AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
		return CodecError::OutOfMemory;
    }

    int ret = av_read_frame(handle, packet);
    if (ret < 0)
    {
        av_packet_free(&packet);
        if (ret == AVERROR_EOF)
        {
			*pCodedFrame = {(AVPacket *)nullptr};
			return CodecError::EndOfFile;
        }
		CLOG_ERROR("Failed to read frame: {}", AVERR_STR(ret));
        return CodecError::EndOfFile;
    }

  //  if (!(packet->flags & AV_PKT_FLAG_KEY))
  //  {
		//av_packet_free(&packet);
		//return CodecError::Again;
  //  }

	auto index = packet->stream_index;
    if (index != streamIndex[(int)MediaType::Video] &&
        index != streamIndex[(int)MediaType::Audio] &&
	    index != streamIndex[(int)MediaType::Subtitle])
    {
		av_packet_free(&packet);
        return CodecError::Again;
    }

	auto stream = handle->streams[index];
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

CodecError FFFormat::Write(const CodedFrame &codedFrame, int stream)
{
	auto packet = codedFrame.InterpretAs<AVPacket>();
    packet->stream_index = stream;

    av_packet_rescale_ts(packet, codecs[stream]->time_base, streams[stream]->time_base);
	int ret = av_interleaved_write_frame(handle, packet);
    if (ret < 0)
    {
		char err[64] = {};
		CLOG_ERROR("Error while writing output packet : {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
    }

    return CodecError::Success;
}

CodecError FFFormat::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	int index = streamIndex[(int)type];
	if (index < 0)
	{
		return CodecError::InvalidArguments;
	}

	auto stream = handle->streams[index];
	if (avformat_seek_file(handle, index, min, pts + stream->start_time, max, AVSEEK_FLAG_BACKWARD) < 0)
	{
		return CodecError::ExternalFailed;
	}

	return CodecError::Success;
}

CodecError FFFormat::GetStreamInfo(MediaType type, CodecInfo &streamInfo)
{
	int index = streamIndex[int(type)];
    if (index < 0)
    {
		return CodecError::NotFound;
    }

	auto stream = handle->streams[index];
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

void FFFormat::EnumerateTracks(MediaType mediaType, std::vector<TrackInfo> &tracks)
{
	tracks.resize(0);
	for (int i = 0; i < handle->nb_streams; i++)
	{
		auto &stream = handle->streams[i];
		if (stream->codecpar->codec_type == (AVMediaType)mediaType)
		{
			AVDictionaryEntry *tag = av_dict_get(stream->metadata, "title", nullptr, 0);
			tracks.emplace_back(TrackInfo{
			    .name = tag ? tag->value : std::string("Track#") + std::to_string(i) + std::string(": ") + avcodec_get_name(stream->codecpar->codec_id),
			    .streamIndex = i});
		}
	}
}

CodecError FFFormat::SwitchTrack(MediaType mediaType, int index)
{
    if (mediaType > MediaType::Subtitle)
    {
		return CodecError::InvalidArguments;
    }

	streamIndex[int(mediaType)] = index;

    return CodecError::Success;
}

Animator &FFFormat::GetAnimator(MediaType mediaType)
{
	static Animator empty{};
	auto index = streamIndex[int(mediaType)];
	if (index >= 0)
	{
		return animators[index];
	}

	return empty;
}

#endif

const String &FFFormat::GetSource() const
{
    return filepath;
}

}
}
