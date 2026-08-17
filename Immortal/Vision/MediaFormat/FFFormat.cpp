#include "FFFormat.h"
#include "Config.h"
#include "Vision/Video/FFCodec.h"
#include "FileSystem/FileSystem.h"
#include "Algorithm/LightVector.h"
#include "Helper/Platform.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <list>
#include <cmath>
#include <limits>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/display.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libavutil/samplefmt.h>
}
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_FFMPEG

#define AVERR_STR(ret) av_make_error_string(err, 64, ret)

static String FFmpegFormatErrorText(const char *prefix, int ret)
{
	char err[64] = {};
	return String{ prefix, StringEncoding::ASCII } + String{ av_make_error_string(err, sizeof(err), ret), StringEncoding::UTF8 };
}

#ifdef _WIN32
struct Win32AVIOOpaque
{
	HANDLE file = INVALID_HANDLE_VALUE;
	String path;
	int64_t fileSize = -1;
	int readErrorLogs = 0;
	int seekErrorLogs = 0;
};

static bool IsWin32FilePath(const String &path)
{
	const char *text = path.c_str();
	const size_t size = path.size();
	if (size >= 2 && text[0] == '\\' && text[1] == '\\')
	{
		return true;
	}
	if (size >= 3 && ((text[0] >= 'A' && text[0] <= 'Z') || (text[0] >= 'a' && text[0] <= 'z')) && text[1] == ':' && (text[2] == '\\' || text[2] == '/'))
	{
		return true;
	}
	return false;
}

static int Win32AVIORead(void *opaque, uint8_t *buffer, int bufferSize)
{
	auto *context = static_cast<Win32AVIOOpaque *>(opaque);
	if (!context || context->file == INVALID_HANDLE_VALUE)
	{
		return AVERROR(EIO);
	}

	DWORD bytesRead = 0;
	if (!ReadFile(context->file, buffer, static_cast<DWORD>(bufferSize), &bytesRead, nullptr))
	{
		DWORD error = GetLastError();
		if (error == ERROR_HANDLE_EOF)
		{
			return AVERROR_EOF;
		}
		context->readErrorLogs++;
		return AVERROR(EIO);
	}
	if (bytesRead == 0)
	{
		return AVERROR_EOF;
	}
	return static_cast<int>(bytesRead);
}

static int64_t Win32AVIOSeek(void *opaque, int64_t offset, int whence)
{
	auto *context = static_cast<Win32AVIOOpaque *>(opaque);
	if (!context || context->file == INVALID_HANDLE_VALUE)
	{
		return AVERROR(EIO);
	}

	if ((whence & ~AVSEEK_FORCE) == AVSEEK_SIZE)
	{
		if (context->fileSize >= 0)
		{
			return context->fileSize;
		}
		return AVERROR(ENOSYS);
	}

	DWORD moveMethod = FILE_BEGIN;
	switch (whence & ~AVSEEK_FORCE)
	{
	case SEEK_SET:
		moveMethod = FILE_BEGIN;
		break;

	case SEEK_CUR:
		moveMethod = FILE_CURRENT;
		break;

	case SEEK_END:
		moveMethod = FILE_END;
		break;

	default:
		return AVERROR(EINVAL);
	}

	LARGE_INTEGER distance{};
	distance.QuadPart = offset;
	LARGE_INTEGER position{};
	if (!SetFilePointerEx(context->file, distance, &position, moveMethod))
	{
		context->seekErrorLogs++;
		return AVERROR(EIO);
	}
	return position.QuadPart;
}
#endif

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

#if LIBAVCODEC_VERSION_MAJOR >= 62
    case AV_CODEC_ID_APV:
        return CodecId::APV;
#endif

    case AV_CODEC_ID_AAC:
        return CodecId::AAC;

    case AV_CODEC_ID_MP3:
        return CodecId::MP3;

    case AV_CODEC_ID_OPUS:
        return CodecId::OPUS;

    case AV_CODEC_ID_VORBIS:
        return CodecId::VORBIS;

    case AV_CODEC_ID_FLAC:
        return CodecId::FLAC;

    case AV_CODEC_ID_PCM_S16LE:
        return CodecId::PCM_S16;

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

static void SetDefaultInputOpenOptions(AVDictionary **options)
{
	// ffplay-style probing budget for MP4/MOV files with late SPS/PPS or sparse indexes.
	av_dict_set(options, "probesize", "67108864", 0);
	av_dict_set(options, "analyzeduration", "10000000", 0);

	// The MP3 demuxer does not populate its Xing/Info seek table by default.
	// Unknown private options remain in the dictionary for other demuxers and
	// are freed by the caller, so this only changes raw MP2/MP3 inputs.
	av_dict_set(options, "usetoc", "1", 0);
}

static void SetDirectShowInputOpenOptions(AVDictionary **options)
{
	// FFmpeg's roughly 3 MiB default can be smaller than a single high-resolution raw frame.
	// Keep enough scheduling headroom for live capture without applying this memory budget to files.
	av_dict_set(options, "rtbufsize", "67108864", 0);
}

static bool IsRawMp3Input(const AVFormatContext *context)
{
	return context && context->iformat && context->iformat->name &&
		std::strcmp(context->iformat->name, "mp3") == 0;
}

static std::string FormatInteger(int64_t value)
{
	return std::to_string(value);
}

static std::string FormatBitrate(int64_t bitRate)
{
	if (bitRate <= 0)
	{
		return {};
	}

	if (bitRate >= 1000000)
	{
		char buffer[64] = {};
		std::snprintf(buffer, sizeof(buffer), "%.3f Mb/s", double(bitRate) / 1000000.0);
		return buffer;
	}
	if (bitRate >= 1000)
	{
		char buffer[64] = {};
		std::snprintf(buffer, sizeof(buffer), "%.0f kb/s", double(bitRate) / 1000.0);
		return buffer;
	}
	return std::to_string(bitRate) + " b/s";
}

static std::string FormatDurationSeconds(double seconds)
{
	if (!std::isfinite(seconds) || seconds < 0.0)
	{
		return {};
	}

	const int64_t milliseconds = int64_t(seconds * 1000.0 + 0.5);
	const int64_t hours = milliseconds / 3600000;
	const int64_t minutes = (milliseconds / 60000) % 60;
	const int64_t wholeSeconds = (milliseconds / 1000) % 60;
	const int64_t ms = milliseconds % 1000;
	char buffer[64] = {};
	std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld.%03lld",
		(long long)hours,
		(long long)minutes,
		(long long)wholeSeconds,
		(long long)ms);
	return buffer;
}

static std::string FormatAVTime(int64_t timestamp, AVRational timebase)
{
	if (timestamp == AV_NOPTS_VALUE || timebase.den == 0)
	{
		return {};
	}
	return FormatDurationSeconds(double(timestamp) * av_q2d(timebase));
}

static std::string FormatRational(AVRational rational)
{
	if (rational.num == 0 || rational.den == 0)
	{
		return {};
	}
	return std::to_string(rational.num) + "/" + std::to_string(rational.den);
}

static bool IsValidRational(AVRational rational)
{
	return rational.num > 0 && rational.den > 0;
}

static AVRational ResolveSampleAspectRatio(const AVFormatContext *format, const AVStream *stream, bool fallbackToSquare = true)
{
	if (!stream)
	{
		return fallbackToSquare ? AVRational{ 1, 1 } : AVRational{};
	}

	AVRational sar = av_guess_sample_aspect_ratio(
		const_cast<AVFormatContext *>(format),
		const_cast<AVStream *>(stream),
		nullptr);
	if (!IsValidRational(sar) && stream->codecpar)
	{
		sar = stream->codecpar->sample_aspect_ratio;
	}
	if (!IsValidRational(sar))
	{
		return fallbackToSquare ? AVRational{ 1, 1 } : AVRational{};
	}
	av_reduce(&sar.num, &sar.den, sar.num, sar.den, std::numeric_limits<int>::max());
	return sar;
}

static std::string FormatRate(AVRational rational, const char *suffix)
{
	if (rational.num == 0 || rational.den == 0)
	{
		return {};
	}

	char buffer[96] = {};
	std::snprintf(buffer, sizeof(buffer), "%s (%.3f %s)", FormatRational(rational).c_str(), av_q2d(rational), suffix);
	return buffer;
}

static const char *MediaTypeDisplayName(AVMediaType type)
{
	switch (type)
	{
	case AVMEDIA_TYPE_VIDEO:
		return "Video";

	case AVMEDIA_TYPE_AUDIO:
		return "Audio";

	case AVMEDIA_TYPE_DATA:
		return "Data";

	case AVMEDIA_TYPE_SUBTITLE:
		return "Subtitle";

	case AVMEDIA_TYPE_ATTACHMENT:
		return "Attachment";

	default:
		break;
	}

	const char *name = av_get_media_type_string(type);
	return name && name[0] ? name : "Unknown";
}

static std::string FormatCodecTag(uint32_t codecTag)
{
	if (!codecTag)
	{
		return {};
	}

	char tag[AV_FOURCC_MAX_STRING_SIZE] = {};
	av_fourcc_make_string(tag, codecTag);
	return std::string{ tag } + " / 0x" + [&] {
		char hex[16] = {};
		std::snprintf(hex, sizeof(hex), "%08x", codecTag);
		return std::string{ hex };
	}();
}

static std::string FormatStreamTitle(const AVFormatContext *handle, const AVStream *stream)
{
	std::string title = "Stream #0:" + std::to_string(stream->index);
	if (stream->id >= 0)
	{
		char id[32] = {};
		std::snprintf(id, sizeof(id), "[0x%x]", stream->id);
		title += id;
	}
	title += " ";
	title += MediaTypeDisplayName(stream->codecpar->codec_type);

	AVDictionaryEntry *language = av_dict_get(stream->metadata, "language", nullptr, 0);
	if (language && language->value && language->value[0])
	{
		title += " (";
		title += language->value;
		title += ")";
	}
	if (stream->disposition & AV_DISPOSITION_DEFAULT)
	{
		title += " default";
	}
	(void)handle;
	return title;
}

static void AddMetadataProperty(FFFormat::MetadataSection &section, const char *name, const std::string &value)
{
	if (value.empty())
	{
		return;
	}
	section.properties.emplace_back(FFFormat::MetadataProperty{
		String{ name, StringEncoding::UTF8 },
		String{ value, StringEncoding::UTF8 }
	});
}

static void AddMetadataProperty(FFFormat::MetadataSection &section, const char *name, const char *value)
{
	if (!value || !value[0])
	{
		return;
	}
	section.properties.emplace_back(FFFormat::MetadataProperty{
		String{ name, StringEncoding::UTF8 },
		String{ value, StringEncoding::UTF8 }
	});
}

static void AddMetadataDictionary(FFFormat::MetadataSection &section, AVDictionary *metadata)
{
	AVDictionaryEntry *tag = nullptr;
	while ((tag = av_dict_get(metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		if (tag->key && tag->value)
		{
			AddMetadataProperty(section, tag->key, tag->value);
		}
	}
}

static void AddDisplayMatrixMetadata(FFFormat::MetadataSection &section, const AVStream *stream)
{
	const AVPacketSideData *sideData = av_packet_side_data_get(
		stream->codecpar->coded_side_data,
		stream->codecpar->nb_coded_side_data,
		AV_PKT_DATA_DISPLAYMATRIX);
	if (!sideData || !sideData->data || sideData->size < 9 * sizeof(int32_t))
	{
		return;
	}

	const double rotation = av_display_rotation_get(reinterpret_cast<const int32_t *>(sideData->data));
	if (!std::isfinite(rotation))
	{
		return;
	}

	char buffer[64] = {};
	std::snprintf(buffer, sizeof(buffer), "%.2f degrees", rotation);
	AddMetadataProperty(section, "Display Matrix Rotation", buffer);
}

static FFFormat::MetadataSection BuildFormatMetadataSection(const AVFormatContext *handle)
{
	FFFormat::MetadataSection section;
	section.title = "Format";
	if (handle->iformat)
	{
		AddMetadataProperty(section, "Format Name", handle->iformat->name);
		AddMetadataProperty(section, "Format Long Name", handle->iformat->long_name);
	}
	AddMetadataProperty(section, "Duration", FormatAVTime(handle->duration, AVRational{ 1, AV_TIME_BASE }));
	AddMetadataProperty(section, "Start Time", FormatAVTime(handle->start_time, AVRational{ 1, AV_TIME_BASE }));
	AddMetadataProperty(section, "Bitrate", FormatBitrate(handle->bit_rate));
	AddMetadataProperty(section, "Stream Count", FormatInteger(handle->nb_streams));
	AddMetadataDictionary(section, handle->metadata);
	return section;
}

static FFFormat::MetadataSection BuildStreamMetadataSection(const AVFormatContext *handle, const AVStream *stream)
{
	const AVCodecParameters *codecpar = stream->codecpar;
	FFFormat::MetadataSection section;
	section.title = FormatStreamTitle(handle, stream);

	AddMetadataProperty(section, "Type", MediaTypeDisplayName(codecpar->codec_type));
	AddMetadataProperty(section, "Codec", avcodec_get_name(codecpar->codec_id));
	AddMetadataProperty(section, "Codec Tag", FormatCodecTag(codecpar->codec_tag));

	const char *profile = avcodec_profile_name(codecpar->codec_id, codecpar->profile);
	AddMetadataProperty(section, "Profile", profile);
	AddMetadataProperty(section, "Bitrate", FormatBitrate(codecpar->bit_rate));
	AddMetadataProperty(section, "Time Base", FormatRational(stream->time_base));
	AddMetadataProperty(section, "Start Time", FormatAVTime(stream->start_time, stream->time_base));
	AddMetadataProperty(section, "Duration", FormatAVTime(stream->duration, stream->time_base));

	switch (codecpar->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
	{
		const AVRational sampleAspectRatio = ResolveSampleAspectRatio(handle, stream);
		AddMetadataProperty(section, "Resolution", std::to_string(codecpar->width) + "x" + std::to_string(codecpar->height));
		AddMetadataProperty(section, "Pixel Format", av_get_pix_fmt_name(static_cast<AVPixelFormat>(codecpar->format)));
		AddMetadataProperty(section, "Sample Aspect Ratio", FormatRational(sampleAspectRatio));
		if (sampleAspectRatio.num > 0 && sampleAspectRatio.den > 0 && codecpar->height > 0)
		{
			const int64_t darNum = int64_t(codecpar->width) * sampleAspectRatio.num;
			const int64_t darDen = int64_t(codecpar->height) * sampleAspectRatio.den;
			AVRational dar{};
			av_reduce(&dar.num, &dar.den, darNum, darDen, std::numeric_limits<int>::max());
			AddMetadataProperty(section, "Display Aspect Ratio", FormatRational(dar));
		}
		AddMetadataProperty(section, "Frame Rate", FormatRate(stream->r_frame_rate, "fps"));
		AddMetadataProperty(section, "Average Frame Rate", FormatRate(stream->avg_frame_rate, "fps"));
		AddMetadataProperty(section, "Color Range", av_color_range_name(codecpar->color_range));
		AddMetadataProperty(section, "Color Space", av_color_space_name(codecpar->color_space));
		AddMetadataProperty(section, "Color Primaries", av_color_primaries_name(codecpar->color_primaries));
		AddMetadataProperty(section, "Transfer Characteristics", av_color_transfer_name(codecpar->color_trc));
		AddDisplayMatrixMetadata(section, stream);
		break;
	}

	case AVMEDIA_TYPE_AUDIO:
		AddMetadataProperty(section, "Sample Rate", codecpar->sample_rate > 0 ? std::to_string(codecpar->sample_rate) + " Hz" : std::string{});
		AddMetadataProperty(section, "Sample Format", av_get_sample_fmt_name(static_cast<AVSampleFormat>(codecpar->format)));
		if (codecpar->ch_layout.nb_channels > 0)
		{
			char layout[256] = {};
			if (av_channel_layout_describe(&codecpar->ch_layout, layout, sizeof(layout)) >= 0)
			{
				AddMetadataProperty(section, "Channel Layout", layout);
			}
			AddMetadataProperty(section, "Channels", FormatInteger(codecpar->ch_layout.nb_channels));
		}
		break;

	default:
		break;
	}

	AddMetadataDictionary(section, stream->metadata);
	return section;
}

FFFormat::FFFormat() :
    handle{},
    customIO{},
    customIOOpaque{},
    streamIndex{-1,-1,-1,-1},
    animators{}
{

}

FFFormat::~FFFormat()
{
	if (handle || customIO || customIOOpaque)
	{
		Close();
	}
}

void FFFormat::CloseCustomIO()
{
#ifdef _WIN32
	if (handle && handle->pb == customIO)
	{
		handle->pb = nullptr;
	}

	if (customIO)
	{
		avio_context_free(&customIO);
	}

	auto *context = static_cast<Win32AVIOOpaque *>(customIOOpaque);
	if (context)
	{
		if (context->file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(context->file);
			context->file = INVALID_HANDLE_VALUE;
		}
		delete context;
		customIOOpaque = nullptr;
	}
#else
	customIO = nullptr;
	customIOOpaque = nullptr;
#endif
}

int FFFormat::OpenWithWin32FileIO(const AVInputFormat *inputFormat, AVDictionary **options)
{
#ifdef _WIN32
	if (!IsWin32FilePath(filepath))
	{
		return AVERROR(EINVAL);
	}

	auto *context = new (std::nothrow) Win32AVIOOpaque{};
	if (!context)
	{
		return AVERROR(ENOMEM);
	}

	std::wstring path = filepath.GetWString();
	context->path = filepath;
	context->file = CreateFileW(
		path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (context->file == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		if (error == ERROR_FILE_TOO_LARGE)
		{
			CLOG_ERROR("Windows WebDAV refused {} because the file exceeds the WebClient FileSizeLimitInBytes limit. Use an HTTP/WebDAV URL or a non-WebClient mount for large files.", filepath);
		}
		else
		{
			CLOG_WARN("Win32AVIO CreateFileW failed for {} - Win32 error {}", filepath, error);
		}
		delete context;
		return AVERROR(EIO);
	}

	LARGE_INTEGER fileSize{};
	if (GetFileSizeEx(context->file, &fileSize))
	{
		context->fileSize = fileSize.QuadPart;
	}
	else
	{
		CLOG_WARN("Win32AVIO GetFileSizeEx failed for {} - Win32 error {}. Continuing with unknown size.", filepath, GetLastError());
	}

	constexpr int kBufferSize = 64 * 1024;
	auto *buffer = static_cast<unsigned char *>(av_malloc(kBufferSize));
	if (!buffer)
	{
		CloseHandle(context->file);
		delete context;
		return AVERROR(ENOMEM);
	}

	customIO = avio_alloc_context(buffer, kBufferSize, 0, context, Win32AVIORead, nullptr, Win32AVIOSeek);
	if (!customIO)
	{
		av_free(buffer);
		CloseHandle(context->file);
		delete context;
		return AVERROR(ENOMEM);
	}

	customIO->seekable = AVIO_SEEKABLE_NORMAL;
	customIOOpaque = context;
	handle->pb = customIO;
	handle->flags |= AVFMT_FLAG_CUSTOM_IO;

	return avformat_open_input(&handle, filepath.c_str(), inputFormat, options);
#else
	(void)inputFormat;
	(void)options;
	return AVERROR(ENOSYS);
#endif
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
		animator->Duration = handle->duration != AV_NOPTS_VALUE
			? (double)handle->duration / (double)AV_TIME_BASE
			: -1.0;

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
			animator->DurationRational = Rational(num, den);
			animator->DurationRational.Reduce();
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
	const bool directShowInput = filepath.size() > 6 && !memcmp(filepath.c_str(), "video=", 6);
	const AVInputFormat *inputFormat = nullptr;
	if (directShowInput)
	{
		avdevice_register_all();
		inputFormat = av_find_input_format("dshow");
	}

	AVDictionary *openOpts = nullptr;
	SetDefaultInputOpenOptions(&openOpts);
	if (directShowInput)
	{
		SetDirectShowInputOpenOptions(&openOpts);
	}

	int ret = avformat_open_input(&handle, filepath.c_str(), inputFormat, &openOpts);
	av_dict_free(&openOpts);
#ifdef _WIN32
	if (ret < 0 && IsWin32FilePath(filepath))
	{
		CLOG_WARN("FFmpeg direct open failed for {} - {}. Retrying with Win32 file IO.", filepath, AVERR_STR(ret));
		if (handle)
		{
			avformat_free_context(handle);
			handle = nullptr;
		}

		handle = avformat_alloc_context();
		if (!handle)
		{
			CLOG_ERROR("Failed to allocated memory for AVFormatContext");
			return CodecError::OutOfMemory;
		}

		openOpts = nullptr;
		SetDefaultInputOpenOptions(&openOpts);
		ret = OpenWithWin32FileIO(inputFormat, &openOpts);
		av_dict_free(&openOpts);
	}
#endif
	if (ret < 0)
	{
		CLOG_ERROR("Failed to open {} - {}", filepath, AVERR_STR(ret));
		CloseCustomIO();
		if (handle)
		{
			avformat_free_context(handle);
			handle = nullptr;
		}
		return CodecError::ExternalFailed;
	}

	// Xing/Info provides a coarse VBR table and `usetoc` turns it into an
	// FFmpeg index. FAST_SEEK additionally gives CBR and legacy MP3 files
	// without a TOC a byte-scaled, frame-synchronised fallback instead of a
	// potentially full-file linear scan.
	if (IsRawMp3Input(handle))
	{
		handle->flags |= AVFMT_FLAG_FAST_SEEK;
	}

	auto options = GenerateStreamInfo(handle);
	ret = avformat_find_stream_info(handle, options);
	if (ret < 0)
	{
		CLOG_ERROR("Failed to find stream info {} - {}", filepath, AVERR_STR(ret));
	}

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
    case CodecId::AVIF:
	case CodecId::AV1_NVENC:
	case CodecId::AV1_QSV:
		return AV_CODEC_ID_AV1;
#if LIBAVCODEC_VERSION_MAJOR >= 62
    case CodecId::APV:
		return AV_CODEC_ID_APV;
#endif

    case CodecId::AAC:
		return AV_CODEC_ID_AAC;

    case CodecId::MP3:
		return AV_CODEC_ID_MP3;

    case CodecId::OPUS:
		return AV_CODEC_ID_OPUS;

    case CodecId::VORBIS:
		return AV_CODEC_ID_VORBIS;

    case CodecId::FLAC:
		return AV_CODEC_ID_FLAC;

    case CodecId::PCM_S16:
		return AV_CODEC_ID_PCM_S16LE;

    default:
        return AV_CODEC_ID_NONE;
    }
}

CodecError FFFormat::Open(const String &_filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec)
{
	int ret = 0;
	char err[64] = {};
	lastError = String{};

    const char *filepath = _filepath.c_str();
	avformat_alloc_output_context2(&handle, nullptr, nullptr, filepath);
    if (!handle)
    {
		lastError = String{ "Failed to allocate output format context.", StringEncoding::ASCII };
		CLOG_ERROR("Failed to alloc output context2 for format context");
		return CodecError::ExternalFailed;
    }

    if (!pCodec)
    {
		lastError = String{ "Invalid output codec list.", StringEncoding::ASCII };
		return CodecError::InvalidArguments;
    }

    auto fmt = handle->oformat;
	for (uint32_t i = 0; i < numCodec; i++)
    {
		FFCodec *ffCodec = InterpretAs<FFCodec>(pCodec[i]);
		if (!ffCodec || !ffCodec->GetHandle())
		{
			const String codecError = ffCodec ? ffCodec->LastError() : String{};
			lastError = codecError.empty() ? String{ "Invalid output codec context.", StringEncoding::ASCII } : codecError;
			return CodecError::InvalidArguments;
		}

		AVCodecContext *codec = ffCodec->GetHandle();
		codecs[i] = codec;
		AVStream *stream  = avformat_new_stream(handle, NULL);
		if (!stream)
		{
			lastError = String{ "Failed to create output stream.", StringEncoding::ASCII };
			return CodecError::OutOfMemory;
		}
		stream->id        = handle->nb_streams - 1;
		stream->time_base = codec->time_base;

		int ret = avcodec_parameters_from_context(stream->codecpar, codec);
        if (ret < 0)
        {
			lastError = FFmpegFormatErrorText("Failed to copy codec parameters from codec: ", ret);
			CLOG_ERROR("Failed to copy codec parameters from codec");
			return CodecError::ExternalFailed;
        }

		if (pCodec[i]->GetMediaType() == MediaType::Video)
		{
			stream->avg_frame_rate = codec->framerate;
			if (codec->codec_id == AV_CODEC_ID_VP9 &&
			    (codec->pix_fmt == AV_PIX_FMT_YUVA420P || stream->codecpar->format == AV_PIX_FMT_YUVA420P))
			{
				av_dict_set(&stream->metadata, "alpha_mode", "1", 0);
			}

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
			lastError = FFmpegFormatErrorText("Could not open output file: ", ret);
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

		lastError = FFmpegFormatErrorText("Error occurred when opening output file for writing header: ", ret);
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
		CloseCustomIO();
		return;
    }

	if (handle->oformat)
	{
		ret = av_write_trailer(handle);
		if (ret < 0)
		{
			lastError = FFmpegFormatErrorText("Error writing trailer: ", ret);
			CLOG_ERROR("Error writing trailer: {}", AVERR_STR(ret));
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
		CloseCustomIO();
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
		if (ret == AVERROR(EAGAIN) || ret == AVERROR(EINTR))
		{
			return CodecError::Again;
		}
		CLOG_ERROR("Failed to read frame: {}", AVERR_STR(ret));
        return CodecError::ExternalFailed;
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

	// Own a detached copy: demux + async decode can otherwise race with buffer lifetime (av_read_frame /
	// avcodec_send_packet may unref data the CodedFrame still points at via cached packet->data).
	AVPacket *owned = av_packet_clone(packet);
	if (!owned)
	{
		av_packet_free(&packet);
		return CodecError::OutOfMemory;
	}
	av_packet_free(&packet);
	packet = owned;

	CodedFrame codedFrame{ packet->data, (size_t)packet->size };
	codedFrame.SetAnonymous(packet);

	codedFrame.SetTimestamp(packet->pts);
	codedFrame.SetType((MediaType) stream->codecpar->codec_type);
	packet->time_base = stream->time_base;

	codedFrame.SetRelease([p = packet](void *) {
		AVPacket *pkt = p;
		av_packet_free(&pkt);
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
		lastError = FFmpegFormatErrorText("Error while writing output packet: ", ret);
		CLOG_ERROR("Error while writing output packet : {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
    }

    return CodecError::Success;
}

CodecError FFFormat::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	if (!handle || type < MediaType::Video || type > MediaType::Subtitle)
	{
		return CodecError::InvalidArguments;
	}

	int index = streamIndex[(int)type];
	if (index < 0 || index >= (int)handle->nb_streams)
	{
		return CodecError::InvalidArguments;
	}

	auto stream = handle->streams[index];
	if (!stream || stream->time_base.num <= 0 || stream->time_base.den <= 0)
	{
		return CodecError::InvalidArguments;
	}

	int64_t startTime = stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time;
	int64_t seekPts = av_sat_add64(pts, startTime);
	int64_t seekMin = min == std::numeric_limits<int64_t>::min() ? min : av_sat_add64(min, startTime);
	int64_t seekMax = max == std::numeric_limits<int64_t>::max() ? max : av_sat_add64(max, startTime);

	int ret = avformat_seek_file(handle, index, seekMin, seekPts, seekMax, AVSEEK_FLAG_BACKWARD);
	if (ret < 0 && IsRawMp3Input(handle))
	{
		// Raw MP3 implements the legacy read_seek callback. Calling it directly
		// is a final fallback for files whose generic min/max seek cannot find an
		// entry (notably MP3 without Xing/VBRI metadata).
		ret = av_seek_frame(handle, index, seekPts, AVSEEK_FLAG_BACKWARD);
	}
	if (ret < 0)
	{
		lastError = FFmpegFormatErrorText("Failed to seek input: ", ret);
		char err[64] = {};
		CLOG_ERROR("Failed to seek {} to pts {} on stream {} - {}", filepath, seekPts, index, AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

	lastError = {};
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
	const AVRational sampleAspectRatio = ResolveSampleAspectRatio(handle, stream, false);
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
		.sampleAspectRatio = { sampleAspectRatio.num, sampleAspectRatio.den },
		.displayOrientation = {}
	};

    if (codecpar->codec_id == AV_CODEC_ID_NONE)
    {
		return CodecError::InvalidArguments;
    }

    return CodecError::Success;
}

CodecError FFFormat::GetMetadata(MetadataSections &sections) const
{
	sections.clear();
	if (!handle)
	{
		return CodecError::InvalidArguments;
	}

	MetadataSection formatSection = BuildFormatMetadataSection(handle);
	if (!formatSection.properties.empty())
	{
		sections.emplace_back(std::move(formatSection));
	}

	for (unsigned int i = 0; i < handle->nb_streams; ++i)
	{
		const AVStream *stream = handle->streams[i];
		if (!stream || !stream->codecpar)
		{
			continue;
		}

		MetadataSection streamSection = BuildStreamMetadataSection(handle, stream);
		if (!streamSection.properties.empty())
		{
			sections.emplace_back(std::move(streamSection));
		}
	}

	return sections.empty() ? CodecError::NotFound : CodecError::Success;
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
	if (!handle)
	{
		return CodecError::InvalidArguments;
	}
	if (index < 0 || index >= (int)handle->nb_streams)
	{
		return CodecError::InvalidArguments;
	}
	AVStream *stream = handle->streams[index];
	if (!stream || !stream->codecpar || stream->codecpar->codec_type != (AVMediaType)mediaType)
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

double FFFormat::GetMaxFrameDurationForSync() const
{
	if (handle && handle->iformat && (handle->iformat->flags & AVFMT_TS_DISCONT))
	{
		return 10.0;
	}
	return 3600.0;
}

#else

CodecError FFFormat::GetMetadata(MetadataSections &sections) const
{
	sections.clear();
	return CodecError::NotImplement;
}

#endif

const String &FFFormat::GetSource() const
{
    return filepath;
}

}
}
