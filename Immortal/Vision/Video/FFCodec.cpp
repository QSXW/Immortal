#include "FFCodec.h"
#include "Vision/MediaFormat/FFFormat.h"
#include "Vision/Processing/ColorSpace.h"
#include "Render/Graphics.h"
#include "Audio/Device.h"
#include "Shared/Log.h"
#include "Vision/Image/ICC.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <list>
#include <string_view>

#if HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
#include <libavutil/display.h>
#include <libavutil/dict.h>
#include <libavutil/pixdesc.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
#ifdef _WIN32
#include <libavutil/hwcontext_d3d12va.h>
#include <d3d12.h>
#endif
}

#include "Shared/Async.h"

namespace Immortal
{
namespace Vision
{

int64_t RationalRescale(int64_t a, Rational bq, Rational cq)
{
	return av_rescale_q(a, {(int)bq.numerator, (int)bq.denominator}, {(int)cq.numerator, (int)cq.denominator});
}

int64_t RationalRescaleRound(int64_t a, int64_t b, int64_t c, int rnd)
{
	return av_rescale_rnd(a, b, c, (AVRounding)rnd);
}

static inline Rational ToRational(const AVRational &value)
{
	return { value.num, value.den };
}

static inline bool IsValidSampleAspectRatio(const Rational &value)
{
	return value.numerator > 0 && value.denominator > 0;
}

static inline Rational ResolveSampleAspectRatio(const AVFrame *frame, const AVCodecContext *context, const Rational &streamValue)
{
	Rational value = streamValue;
	if (frame)
	{
		Rational frameValue = ToRational(frame->sample_aspect_ratio);
		if (!IsValidSampleAspectRatio(value) && IsValidSampleAspectRatio(frameValue))
		{
			value = frameValue;
		}
	}
	if (!IsValidSampleAspectRatio(value) && context)
	{
		value = ToRational(context->sample_aspect_ratio);
	}
	if (!IsValidSampleAspectRatio(value))
	{
		value = { 1, 1 };
	}
	return value;
}

static inline ColorSpace ColorSpaceConverter(AVColorSpace v)
{
    switch (v)
    {
    case AVCOL_SPC_BT470BG:
		return ColorSpace::BT601;

    case AVCOL_SPC_BT2020_NCL:
    case AVCOL_SPC_BT2020_CL:
		return ColorSpace::BT2020;

	case AVCOL_SPC_UNSPECIFIED:
    case AVCOL_SPC_BT709:
    default:
        return ColorSpace::BT709;
    }
}

#define FORMAT_MAPPING                                 \
    CASE(Format::YUV420P,      AV_PIX_FMT_YUV420P  ) \
    CASE(Format::YUVA420P,     AV_PIX_FMT_YUVA420P ) \
    CASE(Format::YUV422P,      AV_PIX_FMT_YUV422P  ) \
    CASE(Format::YUV444P,      AV_PIX_FMT_YUV444P  ) \
    CASE(Format::YUV420P10,    AV_PIX_FMT_YUV420P10) \
    CASE(Format::YUV422P10,    AV_PIX_FMT_YUV422P10) \
    CASE(Format::YUV444P10,    AV_PIX_FMT_YUV444P10) \
    CASE(Format::YUV420P12,    AV_PIX_FMT_YUV420P12) \
	CASE(Format::YUV422P12,    AV_PIX_FMT_YUV422P12) \
	CASE(Format::YUV444P12,    AV_PIX_FMT_YUV444P12) \
    CASE(Format::YUV420P16,    AV_PIX_FMT_YUV420P16) \
	CASE(Format::YUV422P16,    AV_PIX_FMT_YUV422P16) \
	CASE(Format::YUV444P16,    AV_PIX_FMT_YUV444P16) \
    CASE(Format::NV12,         AV_PIX_FMT_NV12     ) \
	CASE(Format::P010,         AV_PIX_FMT_P010     ) \
	CASE(Format::P012,         AV_PIX_FMT_P012     ) \
	CASE(Format::P016,         AV_PIX_FMT_P016     ) \
	CASE(Format::P210,         AV_PIX_FMT_P210     ) \
	CASE(Format::P212,         AV_PIX_FMT_P212     ) \
	CASE(Format::P216,         AV_PIX_FMT_P216     ) \
	CASE(Format::Y210,         AV_PIX_FMT_Y210     ) \
	CASE(Format::Y216,         AV_PIX_FMT_Y216     ) \
	CASE(Format::R8G8B8_UNORM, AV_PIX_FMT_RGB24    ) \
	CASE(Format::B8G8R8_UNORM, AV_PIX_FMT_BGR24    ) \
	CASE(Format::RGBA8,        AV_PIX_FMT_RGBA     ) \
	CASE(Format::BGRA8,        AV_PIX_FMT_BGRA     ) \
	CASE(Format::ARGB,         AV_PIX_FMT_ARGB     ) \
    CASE(Format::RGBA16,       AV_PIX_FMT_RGBA64   ) \
    CASE(Format::R32G32B32A32_SFLOAT, AV_PIX_FMT_RGBAF32)

Format CAST(AVPixelFormat v)
{
#define CASE(y, x) case x: return y;
    switch (v)
    {
		FORMAT_MAPPING
		case AV_PIX_FMT_YUVJ420P:
			return Format::YUV420P;

        default:
            return Format::None;
    }
#undef CASE
}

static bool IsHostBigEndian()
{
    const uint16_t value = 0x0102;
    return *(const uint8_t *)&value == 0x01;
}

static uint32_t ByteSwap32(uint32_t value)
{
    return ((value & 0x000000ffu) << 24) |
        ((value & 0x0000ff00u) << 8) |
        ((value & 0x00ff0000u) >> 8) |
        ((value & 0xff000000u) >> 24);
}

static float ReadFloat32PixelComponent(const uint8_t *data, bool byteSwap)
{
    uint32_t bits = 0;
    memcpy(&bits, data, sizeof(bits));
    if (byteSwap)
    {
        bits = ByteSwap32(bits);
    }

    float value = 0.0f;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static bool IsBigEndianFloat32RgbFormat(AVPixelFormat format)
{
    return format == AV_PIX_FMT_RGBAF32BE ||
        format == AV_PIX_FMT_RGBF32BE ||
        format == AV_PIX_FMT_GBRPF32BE ||
        format == AV_PIX_FMT_GBRAPF32BE;
}

static bool IsLittleEndianFloat32RgbFormat(AVPixelFormat format)
{
    return format == AV_PIX_FMT_RGBAF32LE ||
        format == AV_PIX_FMT_RGBF32LE ||
        format == AV_PIX_FMT_GBRPF32LE ||
        format == AV_PIX_FMT_GBRAPF32LE;
}

static bool Float32RgbFormatNeedsByteSwap(AVPixelFormat format)
{
    if (IsBigEndianFloat32RgbFormat(format))
    {
        return !IsHostBigEndian();
    }
    if (IsLittleEndianFloat32RgbFormat(format))
    {
        return IsHostBigEndian();
    }
    return false;
}

static Picture CopyFloat32RgbFrameToRgba32F(const AVFrame *frame)
{
    if (!frame || frame->width <= 0 || frame->height <= 0)
    {
        return {};
    }

    const AVPixelFormat pixelFormat = (AVPixelFormat)frame->format;
    const bool packedRgba =
        pixelFormat == AV_PIX_FMT_RGBAF32BE ||
        pixelFormat == AV_PIX_FMT_RGBAF32LE;
    const bool packedRgb =
        pixelFormat == AV_PIX_FMT_RGBF32BE ||
        pixelFormat == AV_PIX_FMT_RGBF32LE;
    const bool planarGbr =
        pixelFormat == AV_PIX_FMT_GBRPF32BE ||
        pixelFormat == AV_PIX_FMT_GBRPF32LE;
    const bool planarGbra =
        pixelFormat == AV_PIX_FMT_GBRAPF32BE ||
        pixelFormat == AV_PIX_FMT_GBRAPF32LE;
    if (!packedRgba && !packedRgb && !planarGbr && !planarGbra)
    {
        return {};
    }
    if ((packedRgba || packedRgb) && !frame->data[0])
    {
        return {};
    }
    if ((planarGbr || planarGbra) && (!frame->data[0] || !frame->data[1] || !frame->data[2] || (planarGbra && !frame->data[3])))
    {
        return {};
    }

    Picture picture{ (uint32_t)frame->width, (uint32_t)frame->height, Format::R32G32B32A32_SFLOAT, true };
    const bool byteSwap = Float32RgbFormatNeedsByteSwap(pixelFormat);
    for (int y = 0; y < frame->height; ++y)
    {
        float *dst = (float *)(picture.GetData() + y * picture.GetStride());
        if (packedRgba || packedRgb)
        {
            const uint8_t *src = frame->data[0] + y * frame->linesize[0];
            const size_t srcPixelSize = sizeof(float) * (packedRgba ? 4 : 3);
            for (int x = 0; x < frame->width; ++x)
            {
                const uint8_t *px = src + x * srcPixelSize;
                dst[x * 4 + 0] = ReadFloat32PixelComponent(px + sizeof(float) * 0, byteSwap);
                dst[x * 4 + 1] = ReadFloat32PixelComponent(px + sizeof(float) * 1, byteSwap);
                dst[x * 4 + 2] = ReadFloat32PixelComponent(px + sizeof(float) * 2, byteSwap);
                dst[x * 4 + 3] = packedRgba ? ReadFloat32PixelComponent(px + sizeof(float) * 3, byteSwap) : 1.0f;
            }
            continue;
        }

        const uint8_t *g = frame->data[0] + y * frame->linesize[0];
        const uint8_t *b = frame->data[1] + y * frame->linesize[1];
        const uint8_t *r = frame->data[2] + y * frame->linesize[2];
        const uint8_t *a = planarGbra ? frame->data[3] + y * frame->linesize[3] : nullptr;
        for (int x = 0; x < frame->width; ++x)
        {
            dst[x * 4 + 0] = ReadFloat32PixelComponent(r + x * sizeof(float), byteSwap);
            dst[x * 4 + 1] = ReadFloat32PixelComponent(g + x * sizeof(float), byteSwap);
            dst[x * 4 + 2] = ReadFloat32PixelComponent(b + x * sizeof(float), byteSwap);
            dst[x * 4 + 3] = a ? ReadFloat32PixelComponent(a + x * sizeof(float), byteSwap) : 1.0f;
        }
    }

    return picture;
}

AVPixelFormat CAST(Format format)
{
#define CASE(x, y) case x: return y;
	switch (Format::ValueType(format))
	{
		FORMAT_MAPPING
		default:
			return AV_PIX_FMT_NONE;
	}
#undef CASE
}
#undef FORMAT_MAPPING

#define FORMAT_MAPPING                            \
    CASE(Format::R8_UINT,   AV_SAMPLE_FMT_U8    ) \
    CASE(Format::R16_SINT,  AV_SAMPLE_FMT_S16   ) \
    CASE(Format::R32_SINT,  AV_SAMPLE_FMT_S32   ) \
    CASE(Format::FLOAT,     AV_SAMPLE_FMT_FLT   ) \
    CASE(Format::DOUBLE,    AV_SAMPLE_FMT_DBL   ) \
    CASE(Format::R8_UINTP,  AV_SAMPLE_FMT_U8P   ) \
    CASE(Format::R16_SINTP, AV_SAMPLE_FMT_S16P  ) \
	CASE(Format::R32_SINTP, AV_SAMPLE_FMT_S32P  ) \
	CASE(Format::FLOATP,    AV_SAMPLE_FMT_FLTP  ) \
    CASE(Format::DOUBLEP,   AV_SAMPLE_FMT_DBLP  ) \
	CASE(Format::R64_SINT,  AV_SAMPLE_FMT_S64   ) \
	CASE(Format::R64_SINTP, AV_SAMPLE_FMT_S64P  )

static inline Format CAST(AVSampleFormat v)
{
#define CASE(y, x) case x: return y;
    switch (v)
    {
		FORMAT_MAPPING
        default:
            return Format::None;
    }
#undef CASE
}

static inline AVSampleFormat CastSampleFormat(Format format)
{
#define CASE(x, y) case x: return y;
	switch (Format::ValueType(format))
	{
		FORMAT_MAPPING
		default:
			return AV_SAMPLE_FMT_NONE;
	}
#undef CASE
}
#undef FORMAT_MAPPING

AudioFifo::AudioFifo() :
    handle{}
{

}

int AudioFifo::Allocate(int sampleFormat, int channels, int numSamples)
{
	if (!(handle = av_audio_fifo_alloc((AVSampleFormat)sampleFormat, channels, numSamples)))
	{
		CLOG_ERROR("Could not allocate FIFO");
		return AVERROR_EXIT;
	}

    return 0;
}

int AudioFifo::Allocate(Format format, int channels, int numSamples)
{
	return Allocate(CastSampleFormat(format), channels, numSamples);
}

AudioFifo::~AudioFifo()
{
	if (handle)
    {
		av_audio_fifo_free(handle);
		handle = {};
    }
}

int AudioFifo::Enqueue(uint8_t **convertedInputSamples, int frameSize)
{
	int ret;

	if ((ret = av_audio_fifo_realloc(handle, av_audio_fifo_size(handle) + frameSize)) < 0)
	{
		CLOG_ERROR("Could not reallocate FIFO");
		return ret;
	}

	if (av_audio_fifo_write(handle, (void **) convertedInputSamples,
	                        frameSize) < frameSize)
	{
		CLOG_ERROR("Could not write data to FIFO");
		return AVERROR_EXIT;
	}

	return 0;
}

int AudioFifo::Read(void *const *data, int numSamples) const
{
	return av_audio_fifo_read(handle, data, numSamples);
}

int AudioFifo::Size() const
{
	return av_audio_fifo_size(handle);
}

#define AVERR_STR(ret) av_make_error_string(err, 64, ret)

static String FFmpegErrorText(const char *prefix, int ret)
{
	char err[64] = {};
	return String{ prefix, StringEncoding::ASCII } + String{ av_make_error_string(err, sizeof(err), ret), StringEncoding::UTF8 };
}

static int add_samples_to_fifo(AVAudioFifo *fifo, uint8_t **converted_input_samples, const int frame_size)
{
	int error;

	/* Make the FIFO as large as it needs to be to hold both,
	 * the old and the new samples. */
	if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0)
	{
		fprintf(stderr, "Could not reallocate FIFO\n");
		return error;
	}

	/* Store the new samples in the FIFO buffer. */
	if (av_audio_fifo_write(fifo, (void **) converted_input_samples,
	                        frame_size) < frame_size)
	{
		fprintf(stderr, "Could not write data to FIFO\n");
		return AVERROR_EXIT;
	}
	return 0;
}

double GetDisplayRotation(const int32_t *displaymatrix)
{
	double theta = 0;
    if (displaymatrix)
    {
		theta = -round(av_display_rotation_get(displaymatrix));
    }

	theta -= 360 * floor(theta / 360 + 0.9 / 360);
	if (fabs(theta - 90 * round(theta / 90)) > 2)
	{
		LOG_ERROR("Odd rotation angle.");
	}

	return theta;
}

static AVCodecID Cast(CodecId id)
{
	switch (id)
    {
		case CodecId::AVC:
			return AV_CODEC_ID_H264;
		case CodecId::HEVC:
			return AV_CODEC_ID_HEVC;
		case CodecId::VVC:
			return AV_CODEC_ID_VVC;
		case CodecId::VP9:
			return AV_CODEC_ID_VP9;
		case CodecId::AV1:
		case CodecId::AVIF:
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
		case CodecId::TIFF:
			return AV_CODEC_ID_TIFF;
		case CodecId::PNG:
			return AV_CODEC_ID_PNG;
		case CodecId::WEBP:
			return AV_CODEC_ID_WEBP;
		case CodecId::JPEGXL:
			return AV_CODEC_ID_JPEGXL;
		case CodecId::MJPEG:
			return AV_CODEC_ID_MJPEG;
		case CodecId::JPEG2000:
			return AV_CODEC_ID_JPEG2000;
		case CodecId::DPX:
			return AV_CODEC_ID_DPX;
		case CodecId::TARGA:
			return AV_CODEC_ID_TARGA;
		case CodecId::PCX:
			return AV_CODEC_ID_PCX;
		case CodecId::EXR:
			return AV_CODEC_ID_EXR;
		case CodecId::SGI:
			return AV_CODEC_ID_SGI;
		case CodecId::SUNRASTER:
			return AV_CODEC_ID_SUNRAST;
		case CodecId::JPEGLS:
			return AV_CODEC_ID_JPEGLS;
		case CodecId::FITS:
			return AV_CODEC_ID_FITS;
		case CodecId::IFF_ILBM:
			return AV_CODEC_ID_IFF_ILBM;
		case CodecId::XBM_IMAGE:
			return AV_CODEC_ID_XBM;
		case CodecId::XFACE:
			return AV_CODEC_ID_XFACE;
		case CodecId::QDRAW:
			return AV_CODEC_ID_QDRAW;
		case CodecId::MPEG4:
			return AV_CODEC_ID_MPEG4;
		case CodecId::PCM_S16:
			return AV_CODEC_ID_PCM_S16LE;
		case CodecId::None:
			return AV_CODEC_ID_NONE;
		default:
			return AV_CODEC_ID_NONE;
    }
}

static const char *QueryEncodecById(const CodecId id)
{
    switch (id)
    {
	case CodecId::AAC:
		return "aac";

	case CodecId::MP3:
		return "libmp3lame";

	case CodecId::OPUS:
		return "libopus";

	case CodecId::VORBIS:
		return "libvorbis";

    case CodecId::FLAC:
		return "flac";

    case CodecId::H264:
		return "libx264";

    case CodecId::H264_NVENC:
		return "h264_nvenc";

    case CodecId::H264_QSV:
		return "h264_qsv";

    case CodecId::HEVC:
		return "libx265";

    case CodecId::HEVC_D3D12ENCODE:
		return "hevc_d3d12va";

    case CodecId::HEVC_NVENC:
		return "hevc_nvenc";

    case CodecId::HEVC_QSV:
		return "hevc_qsv";

    case CodecId::VVC:
		return "libvvenc";

    case CodecId::VVC_QSV:
		return "vvc_qsv";

    case CodecId::AV1:
	case CodecId::AVIF:
		return "librav1e";

	case CodecId::AV1_NVENC:
		return "av1_nvenc";

	case CodecId::AV1_QSV:
		return "av1_qsv";

    case CodecId::VP9:
		return "libvpx-vp9";

    case CodecId::VP9_QSV:
		return "vp9_qsv";

    case CodecId::APV:
		return "liboapv";

    case CodecId::TIFF:
		return "tiff";

    case CodecId::PNG:
		return "png";

	case CodecId::WEBP:
		return "libwebp";

	case CodecId::JPEGXL:
		return "libjxl";

	case CodecId::MJPEG:
		return "mjpeg";

	case CodecId::JPEG2000:
		return "jpeg2000";

	case CodecId::DPX:
		return "dpx";

	case CodecId::TARGA:
		return "targa";

	case CodecId::PCX:
		return "pcx";

	case CodecId::EXR:
		return "exr";

	case CodecId::SGI:
		return "sgi";

	case CodecId::SUNRASTER:
		return "sunras";

	case CodecId::JPEGLS:
		return "jpegls";

	case CodecId::FITS:
		return "fits";

	case CodecId::IFF_ILBM:
		return "iff_ilbm";

	case CodecId::XBM_IMAGE:
		return "xbm";

	case CodecId::XFACE:
		return "xface";

	case CodecId::QDRAW:
		return "qdraw";

	case CodecId::MPEG4:
		return "mpeg4";

	case CodecId::PCM_S16:
		return "pcm_s16le";

    default:
        return nullptr;
    }
}

static bool EncoderNameContains(const char *encoderName, std::string_view needle)
{
	if (!encoderName)
	{
		return false;
	}
	return std::string_view{ encoderName }.find(needle) != std::string_view::npos;
}

static int PresetRank(const std::string &preset)
{
	if (preset == "veryslow")
	{
		return 0;
	}
	if (preset == "slower")
	{
		return 1;
	}
	if (preset == "slow")
	{
		return 2;
	}
	if (preset == "medium" || preset.empty())
	{
		return 3;
	}
	if (preset == "faster")
	{
		return 4;
	}
	if (preset == "veryfast")
	{
		return 5;
	}
	if (preset == "superfast")
	{
		return 6;
	}
	if (preset == "ultrafast")
	{
		return 7;
	}
	return 3;
}

static const char *NvencPresetFromRank(int rank)
{
	static constexpr const char *presets[] = { "p7", "p6", "p5", "p4", "p3", "p2", "p1", "p1" };
	return presets[std::clamp(rank, 0, 7)];
}

static const char *QsvPresetFromRank(int rank)
{
	static constexpr const char *presets[] = { "veryslow", "slower", "slow", "medium", "faster", "veryfast", "veryfast", "veryfast" };
	return presets[std::clamp(rank, 0, 7)];
}

static int VpxCpuUsedFromRank(int rank)
{
	static constexpr int values[] = { 0, 1, 2, 4, 5, 6, 7, 8 };
	return values[std::clamp(rank, 0, 7)];
}

static int Rav1eSpeedFromRank(int rank)
{
	static constexpr int values[] = { 0, 2, 3, 5, 7, 8, 9, 10 };
	return values[std::clamp(rank, 0, 7)];
}

static const char *VvencPresetFromRank(int rank)
{
	static constexpr const char *presets[] = { "slower", "slower", "slow", "medium", "fast", "faster", "faster", "faster" };
	return presets[std::clamp(rank, 0, 7)];
}

static const char *OapvPresetFromRank(int rank)
{
	static constexpr const char *presets[] = { "placebo", "placebo", "slow", "medium", "fast", "fastest", "fastest", "fastest" };
	return presets[std::clamp(rank, 0, 7)];
}

static void SetEncoderOption(AVDictionary **options, const char *key, const std::string &value)
{
	if (!value.empty())
	{
		av_dict_set(options, key, value.c_str(), 0);
	}
}

static void SetEncoderOption(AVDictionary **options, const char *key, const char *value)
{
	if (value && value[0])
	{
		av_dict_set(options, key, value, 0);
	}
}

static void SetEncoderOption(AVDictionary **options, const char *key, int value)
{
	av_dict_set(options, key, std::to_string(value).c_str(), 0);
}

static const char *PixelFormatName(AVPixelFormat format)
{
	const char *name = av_get_pix_fmt_name(format);
	return name ? name : "none";
}

static bool PixelFormatHasAlpha(AVPixelFormat format)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(format);
	return desc && (desc->flags & AV_PIX_FMT_FLAG_ALPHA);
}

template <class T>
static bool QueryCodecSupportedConfig(
	const AVCodec *codec,
	AVCodecConfig config,
	const T *&values,
	int &count)
{
	values = nullptr;
	count = 0;
	const void *rawValues = nullptr;
	if (!codec || avcodec_get_supported_config(nullptr, codec, config, 0, &rawValues, &count) < 0)
	{
		return false;
	}
	values = static_cast<const T *>(rawValues);
	return true;
}

static bool CodecSupportsPixelFormat(const AVCodec *codec, AVPixelFormat format)
{
	if (format == AV_PIX_FMT_NONE)
	{
		return false;
	}

	const AVPixelFormat *formats = nullptr;
	int count = 0;
	if (!QueryCodecSupportedConfig(codec, AV_CODEC_CONFIG_PIX_FORMAT, formats, count))
	{
		return false;
	}
	if (!formats)
	{
		return true;
	}
	for (int i = 0; i < count; ++i)
	{
		if (formats[i] == format)
		{
			return true;
		}
	}
	return false;
}

static AVPixelFormat SelectEncoderPixelFormat(const AVCodec *codec, const CodecInfo &encodeInfo, const char *encoderName)
{
	const AVPixelFormat requested = CAST(encodeInfo.format);
	if (CodecSupportsPixelFormat(codec, requested))
	{
		return requested;
	}

	if (requested == AV_PIX_FMT_YUVA420P && EncoderNameContains(encoderName, "libvpx-vp9"))
	{
		// Some FFmpeg builds do not advertise alpha-capable VP9 cleanly. Try the exact
		// requested format so alpha output fails loudly instead of silently dropping to yuv420p.
		return AV_PIX_FMT_YUVA420P;
	}

	if (PixelFormatHasAlpha(requested))
	{
		LOG_ERROR("Encoder '{}' does not support requested alpha pixel format '{}'.", encoderName, PixelFormatName(requested));
		return AV_PIX_FMT_NONE;
	}

	if (EncoderNameContains(encoderName, "liboapv"))
	{
		const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(requested);
		const bool prefer12Bit = desc && desc->nb_components > 0 && desc->comp[0].depth > 10;
		const bool prefer444 = desc &&
			!(desc->flags & AV_PIX_FMT_FLAG_RGB) &&
			desc->nb_components >= 3 &&
			desc->log2_chroma_w == 0 &&
			desc->log2_chroma_h == 0;
		const AVPixelFormat fallback = prefer444
			? (prefer12Bit ? AV_PIX_FMT_YUV444P12 : AV_PIX_FMT_YUV444P10)
			: (prefer12Bit ? AV_PIX_FMT_YUV422P12 : AV_PIX_FMT_YUV422P10);
		if (CodecSupportsPixelFormat(codec, fallback))
		{
			return fallback;
		}
	}

	const AVPixelFormat *formats = nullptr;
	int count = 0;
	return QueryCodecSupportedConfig(codec, AV_CODEC_CONFIG_PIX_FORMAT, formats, count) && formats && count > 0
		? formats[0]
		: AV_PIX_FMT_NONE;
}

static void ApplyVideoEncoderOptions(AVCodecContext *handle, AVDictionary **options, const char *encoderName, const CodecInfo &encodeInfo)
{
	if (!handle || handle->codec_type != AVMEDIA_TYPE_VIDEO)
	{
		return;
	}

	if (encodeInfo.hardwareDeviceIndex >= 0 && EncoderNameContains(encoderName, "_nvenc"))
	{
		SetEncoderOption(options, "gpu", encodeInfo.hardwareDeviceIndex);
	}

	const int presetRank = PresetRank(encodeInfo.encoderPreset);
	if (!encodeInfo.encoderPreset.empty())
	{
		if (EncoderNameContains(encoderName, "_nvenc"))
		{
			SetEncoderOption(options, "preset", NvencPresetFromRank(presetRank));
		}
		else if (EncoderNameContains(encoderName, "_qsv"))
		{
			SetEncoderOption(options, "preset", QsvPresetFromRank(presetRank));
		}
		else if (EncoderNameContains(encoderName, "libvpx"))
		{
			SetEncoderOption(options, "cpu-used", VpxCpuUsedFromRank(presetRank));
		}
		else if (EncoderNameContains(encoderName, "librav1e"))
		{
			SetEncoderOption(options, "speed", Rav1eSpeedFromRank(presetRank));
		}
		else if (EncoderNameContains(encoderName, "libvvenc"))
		{
			SetEncoderOption(options, "preset", VvencPresetFromRank(presetRank));
		}
		else if (EncoderNameContains(encoderName, "liboapv"))
		{
			SetEncoderOption(options, "preset", OapvPresetFromRank(presetRank));
		}
		else
		{
			SetEncoderOption(options, "preset", encodeInfo.encoderPreset);
		}
	}

	const std::string &rateControl = encodeInfo.rateControl;
	const bool isVvenc = EncoderNameContains(encoderName, "libvvenc");
	const bool isOapv = EncoderNameContains(encoderName, "liboapv");
	if (isVvenc)
	{
		const int qp = std::clamp(encodeInfo.crf > 0 ? encodeInfo.crf : 32, 0, 63);
		handle->bit_rate = 0;
		handle->rc_min_rate = 0;
		handle->rc_max_rate = 0;
		handle->rc_buffer_size = 0;
		SetEncoderOption(options, "qp", qp);
		return;
	}

	if (rateControl == "cbr")
	{
		handle->bit_rate       = encodeInfo.bitRate;
		handle->rc_min_rate    = encodeInfo.bitRate;
		handle->rc_max_rate    = encodeInfo.maxBitRate > 0 ? encodeInfo.maxBitRate : encodeInfo.bitRate;
		handle->rc_buffer_size = encodeInfo.bufferSize > 0 ? encodeInfo.bufferSize : encodeInfo.bitRate * 2;
		if (EncoderNameContains(encoderName, "_nvenc"))
		{
			SetEncoderOption(options, "rc", "cbr");
		}
		else if (EncoderNameContains(encoderName, "_qsv"))
		{
			SetEncoderOption(options, "rate_control", "cbr");
		}
	}
	else if (rateControl == "vbr")
	{
		handle->bit_rate       = encodeInfo.bitRate;
		handle->rc_min_rate    = 0;
		handle->rc_max_rate    = encodeInfo.maxBitRate > 0 ? encodeInfo.maxBitRate : encodeInfo.bitRate * 2;
		handle->rc_buffer_size = encodeInfo.bufferSize > 0 ? encodeInfo.bufferSize : handle->rc_max_rate * 2;
		if (EncoderNameContains(encoderName, "_nvenc"))
		{
			SetEncoderOption(options, "rc", "vbr");
		}
		else if (EncoderNameContains(encoderName, "_qsv"))
		{
			SetEncoderOption(options, "rate_control", "vbr");
		}
	}
	else if (rateControl == "crf")
	{
		const int quality = std::clamp(encodeInfo.crf > 0 ? encodeInfo.crf : 23, 0, 63);
		handle->bit_rate = 0;
		handle->rc_min_rate = 0;
		handle->rc_max_rate = 0;
		handle->rc_buffer_size = 0;
		if (EncoderNameContains(encoderName, "_nvenc"))
		{
			SetEncoderOption(options, "rc", "vbr");
			SetEncoderOption(options, "cq", quality);
		}
		else if (EncoderNameContains(encoderName, "_qsv"))
		{
			SetEncoderOption(options, "global_quality", quality);
		}
		else if (EncoderNameContains(encoderName, "librav1e"))
		{
			SetEncoderOption(options, "qp", quality);
		}
		else if (isOapv)
		{
			SetEncoderOption(options, "qp", quality);
		}
		else
		{
			SetEncoderOption(options, "crf", quality);
		}
	}
	else
	{
		handle->bit_rate = encodeInfo.bitRate;
	}

	if (isOapv && handle->bit_rate > 0)
	{
		SetEncoderOption(options, "qp", 0);
	}

	if (EncoderNameContains(encoderName, "libvpx") && handle->pix_fmt == AV_PIX_FMT_YUVA420P)
	{
		SetEncoderOption(options, "auto-alt-ref", "0");
	}
}

#define IS_LAYOUT(L) ((&(layout = AV_CHANNEL_LAYOUT_##L)) && (av_channel_layout_compare(&channelLayout, &layout) == 0))
static ChannelLayout CAST(AVChannelLayout &channelLayout)
{
	AVChannelLayout layout;
	if (IS_LAYOUT(MONO))
	{
		return ChannelLayout::Mono;
	}
	else if (IS_LAYOUT(STEREO))
	{
		return ChannelLayout::Stereo;
	}
	else if (IS_LAYOUT(2POINT1))
	{
		return ChannelLayout::_2Point1;
	}
	else if (IS_LAYOUT(2_1))
	{
		return ChannelLayout::_2_1;
	}
	else if (IS_LAYOUT(SURROUND))
	{
		return ChannelLayout::Surround;
	}
	else if (IS_LAYOUT(3POINT1))
	{
		return ChannelLayout::_3Point1;
	}
	else if (IS_LAYOUT(4POINT0))
	{
		return ChannelLayout::_4Point0;
	}
	else if (IS_LAYOUT(4POINT1))
	{
		return ChannelLayout::_4Point1;
	}
	else if (IS_LAYOUT(2_2))
	{
		return ChannelLayout::_2_2;
	}
	else if (IS_LAYOUT(QUAD))
	{
		return ChannelLayout::Quad;
	}
	else if (IS_LAYOUT(5POINT0))
	{
		return ChannelLayout::_5Point0;
	}
	else if (IS_LAYOUT(5POINT1))
	{
		return ChannelLayout::_5Point1;
	}
	else if (IS_LAYOUT(5POINT0_BACK))
	{
		return ChannelLayout::_5Point0Back;
	}
	else if (IS_LAYOUT(5POINT1_BACK))
	{
		return ChannelLayout::_5Point1Back;
	}
	else if (IS_LAYOUT(6POINT0))
	{
		return ChannelLayout::_6Point0;
	}
	else if (IS_LAYOUT(6POINT0_FRONT))
	{
		return ChannelLayout::_6Point0Front;
	}
	else if (IS_LAYOUT(3POINT1POINT2))
	{
		return ChannelLayout::_3Point1Point2;
	}
	else if (IS_LAYOUT(HEXAGONAL))
	{
		return ChannelLayout::Hexagonal;
	}
	else if (IS_LAYOUT(6POINT1))
	{
		return ChannelLayout::_6Point1;
	}
	else if (IS_LAYOUT(6POINT1_BACK))
	{
		return ChannelLayout::_6Point1Back;
	}
	else if (IS_LAYOUT(6POINT1_FRONT))
	{
		return ChannelLayout::_6Point1Front;
	}
	else if (IS_LAYOUT(7POINT0))
	{
		return ChannelLayout::_7Point0;
	}
	else if (IS_LAYOUT(7POINT0_FRONT))
	{
		return ChannelLayout::_7Point0Front;
	}
	else if (IS_LAYOUT(7POINT1))
	{
		return ChannelLayout::_7Point1;
	}
	else if (IS_LAYOUT(7POINT1_WIDE))
	{
		return ChannelLayout::_7Point1Wide;
	}
	else if (IS_LAYOUT(7POINT1_WIDE_BACK))
	{
		return ChannelLayout::_7Point1WideBack;
	}
	else if (IS_LAYOUT(5POINT1POINT2_BACK))
	{
		return ChannelLayout::_5Point1Point2Back;
	}
	else if (IS_LAYOUT(OCTAGONAL))
	{
		return ChannelLayout::Octagonal;
	}
	else if (IS_LAYOUT(CUBE))
	{
		return ChannelLayout::Cube;
	}
	else if (IS_LAYOUT(5POINT1POINT4_BACK))
	{
		return ChannelLayout::_5Point1Point4Back;
	}
	else if (IS_LAYOUT(7POINT1POINT2))
	{
		return ChannelLayout::_7Point1Point2;
	}
	else if (IS_LAYOUT(7POINT1POINT4_BACK))
	{
		return ChannelLayout::_7Point1Point4Back;
	}
	else if (IS_LAYOUT(7POINT2POINT3))
	{
		return ChannelLayout::_7Point2Point3;
	}
	else if (IS_LAYOUT(9POINT1POINT4_BACK))
	{
		return ChannelLayout::_9Point1Point4Back;
	}
	else if (IS_LAYOUT(HEXADECAGONAL))
	{
		return ChannelLayout::Hexadecagonal;
	}
	else if (IS_LAYOUT(STEREO_DOWNMIX))
	{
		return ChannelLayout::StereoDownmix;
	}
	else if (IS_LAYOUT(22POINT2))
	{
		return ChannelLayout::_22Point2;
	}
	else
	{
		return ChannelLayout::Stereo;
	}
}

ChannelLayout GetLayoutFromMask(uint64_t mask)
{
	AVChannelLayout layout;
	int ret = av_channel_layout_from_mask(&layout, mask);
    if (ret < 0)
    {
		return ChannelLayout::Stereo;
    }

    return CAST(layout);
}

SampleConverter::SampleConverter() :
    ICLASS,
    handle{}
{

}

SampleConverter::~SampleConverter()
{
	Release();
}

void SampleConverter::Release()
{
	if (handle)
	{
		swr_close(handle);
		swr_free(&handle);
		handle = nullptr;
	}
}

void SampleConverter::Reset()
{
	Release();
	inputFormat = {};
	outputFormat = {};
	outFormatSize = 0;
	inputFormatSize = 0;
}

static bool GetChannelLayout(AVChannelLayout &out, const AudioFormatSpec &format)
{
    switch (format.layout)
    {
        case ChannelLayout::Mono:
            out = AV_CHANNEL_LAYOUT_MONO;
            break;
        case ChannelLayout::Stereo:
            out = AV_CHANNEL_LAYOUT_STEREO;
            break;
        case ChannelLayout::_2Point1:
            out = AV_CHANNEL_LAYOUT_2POINT1;
            break;
        case ChannelLayout::_2_1:
            out = AV_CHANNEL_LAYOUT_2_1;
            break;
        case ChannelLayout::Surround:
            out = AV_CHANNEL_LAYOUT_SURROUND;
            break;
        case ChannelLayout::_3Point1:
            out = AV_CHANNEL_LAYOUT_3POINT1;
            break;
        case ChannelLayout::_4Point0:
            out = AV_CHANNEL_LAYOUT_4POINT0;
            break;
        case ChannelLayout::_4Point1:
            out = AV_CHANNEL_LAYOUT_4POINT1;
            break;
        case ChannelLayout::_2_2:
            out = AV_CHANNEL_LAYOUT_2_2;
            break;
        case ChannelLayout::Quad:
            out = AV_CHANNEL_LAYOUT_QUAD;
            break;
        case ChannelLayout::_5Point0:
            out = AV_CHANNEL_LAYOUT_5POINT0;
            break;
        case ChannelLayout::_5Point1:
            out = AV_CHANNEL_LAYOUT_5POINT1;
            break;
        case ChannelLayout::_5Point0Back:
            out = AV_CHANNEL_LAYOUT_5POINT0_BACK;
            break;
        case ChannelLayout::_5Point1Back:
            out = AV_CHANNEL_LAYOUT_5POINT1_BACK;
            break;
        case ChannelLayout::_6Point0:
            out = AV_CHANNEL_LAYOUT_6POINT0;
            break;
        case ChannelLayout::_6Point0Front:
            out = AV_CHANNEL_LAYOUT_6POINT0_FRONT;
            break;
        case ChannelLayout::_3Point1Point2:
            out = AV_CHANNEL_LAYOUT_3POINT1POINT2;
            break;
        case ChannelLayout::Hexagonal:
            out = AV_CHANNEL_LAYOUT_HEXAGONAL;
            break;
        case ChannelLayout::_6Point1:
            out = AV_CHANNEL_LAYOUT_6POINT1;
            break;
        case ChannelLayout::_6Point1Back:
            out = AV_CHANNEL_LAYOUT_6POINT1_BACK;
            break;
        case ChannelLayout::_6Point1Front:
            out = AV_CHANNEL_LAYOUT_6POINT1_FRONT;
            break;
        case ChannelLayout::_7Point0:
            out = AV_CHANNEL_LAYOUT_7POINT0;
            break;
        case ChannelLayout::_7Point0Front:
            out = AV_CHANNEL_LAYOUT_7POINT0_FRONT;
            break;
        case ChannelLayout::_7Point1:
            out = AV_CHANNEL_LAYOUT_7POINT1;
            break;
        case ChannelLayout::_7Point1Wide:
            out = AV_CHANNEL_LAYOUT_7POINT1_WIDE;
            break;
        case ChannelLayout::_7Point1WideBack:
            out = AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK;
            break;
        case ChannelLayout::_5Point1Point2Back:
            out = AV_CHANNEL_LAYOUT_5POINT1POINT2_BACK;
            break;
        case ChannelLayout::Octagonal:
            out = AV_CHANNEL_LAYOUT_OCTAGONAL;
            break;
        case ChannelLayout::Cube:
            out = AV_CHANNEL_LAYOUT_CUBE;
            break;
        case ChannelLayout::_5Point1Point4Back:
            out = AV_CHANNEL_LAYOUT_5POINT1POINT4_BACK;
            break;
        case ChannelLayout::_7Point1Point2:
            out = AV_CHANNEL_LAYOUT_7POINT1POINT2;
            break;
        case ChannelLayout::_7Point1Point4Back:
            out = AV_CHANNEL_LAYOUT_7POINT1POINT4_BACK;
            break;
        case ChannelLayout::_7Point2Point3:
            out = AV_CHANNEL_LAYOUT_7POINT2POINT3;
            break;
        case ChannelLayout::_9Point1Point4Back:
            out = AV_CHANNEL_LAYOUT_9POINT1POINT4_BACK;
            break;
        case ChannelLayout::Hexadecagonal:
            out = AV_CHANNEL_LAYOUT_HEXADECAGONAL;
            break;
        case ChannelLayout::StereoDownmix:
            out = AV_CHANNEL_LAYOUT_STEREO_DOWNMIX;
            break;
        case ChannelLayout::_22Point2:
            out = AV_CHANNEL_LAYOUT_22POINT2;
            break;
        default:
            return false;
    }
    return true;
}

bool SampleConverter::SetOptions(const AudioFormatSpec &_outputFormat, const AudioFormatSpec &_inputFormat)
{
	Release();

	outputFormat = _outputFormat;
    inputFormat  = _inputFormat;

	// Reject obviously bogus specs up front. swr_alloc_set_opts2 will happily
	// allocate a context for `sampleRate == 0`, but the subsequent swr_init
	// will fail and leave us with an allocated-but-uninitialized handle. Any
	// later Convert() call would then trip FFmpeg's "Context has not been
	// initialized" assertion, which is the exact symptom we want to avoid.
	if (inputFormat.sampleRate <= 0 || outputFormat.sampleRate <= 0 ||
	    inputFormat.numChannel <= 0 || outputFormat.numChannel <= 0)
	{
		CLOG_ERROR("Invalid SampleConverter spec: input {}Hz/{}ch -> output {}Hz/{}ch",
		           inputFormat.sampleRate, inputFormat.numChannel,
		           outputFormat.sampleRate, outputFormat.numChannel);
		return false;
	}

	AVChannelLayout outChannelLayout;
    if (!GetChannelLayout(outChannelLayout, outputFormat))
    {
		CLOG_ERROR("Error when getting output channel layout - {}", (int)outputFormat.layout);
		return false;
    }

	AVChannelLayout inputChannelLayout;
	if (!GetChannelLayout(inputChannelLayout, inputFormat))
	{
		CLOG_ERROR("Error when getting input channel layout - {}", (int) outputFormat.layout);
		return false;
	}

	int ret = swr_alloc_set_opts2(&handle,
        &outChannelLayout,
        CastSampleFormat(outputFormat.format),
        outputFormat.sampleRate,
        &inputChannelLayout,
        CastSampleFormat(inputFormat.format),
        inputFormat.sampleRate,
        0,
        nullptr);

    if (ret < 0 || swr_init(handle) < 0)
    {
		CLOG_ERROR("Error when init swr context (input {}Hz/{}ch fmt={} -> output {}Hz/{}ch fmt={})",
		           inputFormat.sampleRate, inputFormat.numChannel, (uint32_t)Format::ValueType(inputFormat.format),
		           outputFormat.sampleRate, outputFormat.numChannel, (uint32_t)Format::ValueType(outputFormat.format));
		// The allocator may have produced a handle even though swr_init failed.
		// Tear it down so `operator bool()` (which only checks `handle`) cannot
		// claim the converter is usable, and so Convert() never runs against an
		// uninitialized SwrContext.
		Release();
		return false;
    }

    outFormatSize = outputFormat.format.GetTexelSize() * outChannelLayout.nb_channels;

    inputFormatSize = inputFormat.format.GetTexelSize();
	if (!(inputFormat.format == Format::FLOATP    ||
	      inputFormat.format == Format::FLOAT8P   ||
	      inputFormat.format == Format::FLOAT16P  ||
          inputFormat.format == Format::R8_UINTP  ||
	      inputFormat.format == Format::R16_SINTP ||
	      inputFormat.format == Format::R32_SINTP
        ))
    {
		inputFormatSize *= outChannelLayout.nb_channels;
    }

    return true;
}

int SampleConverter::RescaleRound(int numSamples)
{
    auto &inputSampleRate  = inputFormat.sampleRate;
	auto &outputSampleRate = outputFormat.sampleRate;
    return av_rescale_rnd(
	    swr_get_delay(handle, inputSampleRate) + numSamples,
	    outputSampleRate,
	    inputSampleRate,
	    AV_ROUND_UP);
}

bool SampleConverter::Convert(Picture &out, const Picture &input)
{
	int ret = swr_convert(
        handle,
	    &out.GetData(),
	    out.GetWidth(),
        &input.GetData(),
        input.GetWidth()
    );

    if (ret < 0)
	{
		CLOG_ERROR("Error when converting {} samples to {}", out.GetWidth(), input.GetWidth());
		return false;
    }

    out.SetWidth(ret);

    return true;
}

Picture SampleConverter::GetRemainingSamples()
{
	int samples = swr_get_out_samples(handle, 0);
	if (samples <= 0)
	{
		return {};
	}

	Picture picture{ samples, outputFormat.numChannel, outputFormat.format, true };
	int ret = swr_convert(handle, &picture.GetData(), samples, NULL, 0);
	if (ret <= 0)
	{
		return {};
	}

	picture.SetSampleRate(outputFormat.sampleRate);
	return picture;
}

SampleConverter::operator bool() const
{
	return !!handle;
}

Scaler::Scaler() :
	handle{}
{

}

Scaler::~Scaler()
{
	if (handle)
	{
		sws_freeContext(handle);
		handle = nullptr;
	}
}

void Scaler::Init(uint32_t srcW, uint32_t srcH, int srcFormat, uint32_t dstW, uint32_t dstH, int dstFormat, int flags)
{
	if (handle)
	{
		sws_freeContext(handle);
		handle = nullptr;
	}
	handle = sws_getContext(srcW, srcH, (AVPixelFormat) srcFormat, dstW, dstH, (AVPixelFormat)dstFormat, flags, nullptr, nullptr, nullptr);
}

void Scaler::Init(uint32_t srcW, uint32_t srcH, Format srcFormat, uint32_t dstW, uint32_t dstH, Format dstFormat, Filter filter)
{
	Init(srcW, srcH, (int)CAST(srcFormat), dstW, dstH, (int)CAST(dstFormat), filter == Filter::Bilinear ? SWS_BILINEAR : SWS_POINT);
}

CodecError Scaler::Scale(const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[])
{
	if (!handle)
	{
		LOG_ERROR("Scaler is not initialized!");
		return CodecError::InvalidArguments;
	}

	char err[64];
	int ret = sws_scale(handle, srcSlice, srcStride, srcSliceY, srcSliceH, dst, dstStride);
	if (ret < 0)
	{
		LOG_ERROR("Failed to scale picture - {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

	return CodecError::Success;
}

CodecError Scaler::Scale(Picture &dst, const Picture &src)
{
	return Scale(&src.GetData(), (const int *) &src.GetStride(), 0, src.GetHeight(),
	             &dst.GetData(), (const int *) &dst.GetStride());
}

Scaler::operator bool() const
{
	return !!handle;
}

FFCodec::FFCodec(const char *name) :
    IClass{name},
    VideoCodec{name},
    handle{},
    device{},
    swrContext{},
    scaler{},
    type{PictureMemoryType::System},
    startTimestamp{},
    displayOrientation{},
    sampleAspectRatio{},
    preference{},
    pts{},
    subtitle{},
    lastError{}
{
    frame = av_frame_alloc();
    ThrowIf(!frame, "FFCodec::Failed to allocated memory for frame!")

    memoryResource = new MemoryResource(sizeof(SharedPictureData));
}

FFCodec::FFCodec(CodecId codecId) :
    FFCodec{}
{
	InitializeDecoder(Cast(codecId));
}

FFCodec::FFCodec(const CodecInfo &encodeInfo) :
    FFCodec{"FFmpegEncoder"}
{
	char err[64] = {};
	isEncoder = true;
	lastError = String{};

	auto name = QueryEncodecById(encodeInfo.codecId);
	if (!name)
	{
		lastError = String{ "Failed to query encoder for codec id.", StringEncoding::ASCII };
		CLOG_ERROR("Failed to query encoder for id - '{}'", (int)encodeInfo.codecId);
		return;
	}

	const AVCodec *codec = avcodec_find_encoder_by_name(name);
	if (!codec)
	{
		lastError = String{ "Failed to find encoder by name: ", StringEncoding::ASCII } + String{ name, StringEncoding::UTF8 };
		CLOG_ERROR("Failed to find encoder by name - '{}'", name);
		return;
	}

	if (!(handle = avcodec_alloc_context3(codec)))
	{
		lastError = String{ "Failed to allocate encoder context.", StringEncoding::ASCII };
		CLOG_ERROR("Failed to alloc context for encoder");
		return;
	}

	AVDictionary *codecOptions = nullptr;
    switch (handle->codec_type)
    {
		case AVMEDIA_TYPE_AUDIO:
	    {
            mediaType = MediaType::Audio;
		    auto sampleFormat = CastSampleFormat(encodeInfo.format);
            const AVSampleFormat *sampleFormats = nullptr;
            int sampleFormatCount = 0;
            if (!QueryCodecSupportedConfig(
				codec,
				AV_CODEC_CONFIG_SAMPLE_FORMAT,
				sampleFormats,
				sampleFormatCount))
            {
				lastError = String{ "Failed to query encoder sample formats.", StringEncoding::ASCII };
				avcodec_free_context(&handle);
				return;
			}
            if (!sampleFormats)
			{
				handle->sample_fmt = sampleFormat;
			}
			else
			{
				for (int i = 0; i < sampleFormatCount; ++i)
                {
					if (sampleFormat == sampleFormats[i])
					{
						handle->sample_fmt = sampleFormat;
						break;
					}
                }
            }
            if (handle->sample_fmt != sampleFormat)
            {
				lastError = String{ "Unsupported audio sample format for encoder.", StringEncoding::ASCII };
				CLOG_ERROR("Unsupported sample format - {}", (int)sampleFormat);
				avcodec_free_context(&handle);
			    return;
            }

            const int *sampleRates = nullptr;
            int sampleRateCount = 0;
            if (!QueryCodecSupportedConfig(
				codec,
				AV_CODEC_CONFIG_SAMPLE_RATE,
				sampleRates,
				sampleRateCount))
			{
				lastError = String{ "Failed to query encoder sample rates.", StringEncoding::ASCII };
				avcodec_free_context(&handle);
				return;
			}
            if (!sampleRates)
            {
				handle->sample_rate = encodeInfo.sampleRate;
            }
			else
            {
				for (int i = 0; i < sampleRateCount; ++i)
				{
					if (encodeInfo.sampleRate == sampleRates[i])
					{
						handle->sample_rate = encodeInfo.sampleRate;
						break;
					}
				}
			}

            if (handle->sample_rate != encodeInfo.sampleRate)
		    {
			    lastError = String{ "Unsupported audio sample rate for encoder.", StringEncoding::ASCII };
			    CLOG_ERROR("Unsupported sample rate - {}", encodeInfo.sampleRate);
				avcodec_free_context(&handle);
			    return;
		    }

		    handle->bit_rate    = encodeInfo.bitRate;
            handle->time_base   = { 1, handle->sample_rate };

			AudioFormatSpec spec{
				.layout = encodeInfo.channelLayout,
			};
			if (!GetChannelLayout(handle->ch_layout, spec))
			{
				lastError = String{ "Failed to get encoder channel layout.", StringEncoding::ASCII };
				CLOG_ERROR("Failed to get channel layout");
				avcodec_free_context(&handle);
				return;
			}
	    }
		break;

        case AVMEDIA_TYPE_VIDEO:
        {
            AVPixelFormat pixelFormat = SelectEncoderPixelFormat(codec, encodeInfo, name);
            if (pixelFormat == AV_PIX_FMT_NONE)
            {
				lastError = String{ "The encoder does not support the requested pixel format.", StringEncoding::ASCII };
				avcodec_free_context(&handle);
				return;
            }

			mediaType = MediaType::Video;
			handle->colorspace   = AVCOL_SPC_BT709;
			handle->color_range  = AVCOL_RANGE_JPEG;
            handle->width        = encodeInfo.width;
            handle->height       = encodeInfo.height;
			handle->pix_fmt      = pixelFormat;
            handle->bit_rate     = encodeInfo.bitRate;
            handle->gop_size     = encodeInfo.gopSize;
            handle->time_base    = AVRational{ (int)encodeInfo.timeBase.numerator, (int)encodeInfo.timeBase.denominator };
            handle->framerate    = AVRational{ (int)encodeInfo.framerate.numerator, (int)encodeInfo.framerate.denominator };
            if (EncoderNameContains(name, "libvvenc") && handle->framerate.num > 0 && handle->framerate.den > 0)
            {
                handle->time_base = av_inv_q(handle->framerate);
            }
            if (IsValidSampleAspectRatio(encodeInfo.sampleAspectRatio))
            {
				handle->sample_aspect_ratio = AVRational{ (int)encodeInfo.sampleAspectRatio.numerator, (int)encodeInfo.sampleAspectRatio.denominator };
            }
	        handle->max_b_frames = 1;
			handle->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			handle->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;
			handle->strict_std_compliance |= FF_COMPLIANCE_EXPERIMENTAL;
			displayOrientation   = encodeInfo.displayOrientation;
			ApplyVideoEncoderOptions(handle, &codecOptions, name, encodeInfo);
			CLOG_DEBUG("Opening video encoder '{}' pixel_format='{}' requested_format={} preset='{}' rate_control='{}' bitrate={} maxrate={} bufsize={} crf={}",
				name,
				PixelFormatName(handle->pix_fmt),
				(uint32_t)(Format::ValueType)encodeInfo.format,
				encodeInfo.encoderPreset.c_str(),
				encodeInfo.rateControl.c_str(),
				handle->bit_rate,
				handle->rc_max_rate,
				handle->rc_buffer_size,
				encodeInfo.crf);
        }
		break;

        default:
			lastError = String{ "Unsupported media type for encoder.", StringEncoding::ASCII };
			avcodec_free_context(&handle);
			return;
    }

    int ret = avcodec_open2(handle, codec, &codecOptions);
	if (codecOptions)
	{
		av_dict_free(&codecOptions);
	}
	if (ret < 0)
	{
		lastError = FFmpegErrorText("Could not open codec: ", ret);
		CLOG_ERROR("Could not open codec: {}", AVERR_STR(ret));
		avcodec_free_context(&handle);
		return;
	}
	if (handle->codec_type == AVMEDIA_TYPE_VIDEO && PixelFormatHasAlpha(CAST(encodeInfo.format)) && !PixelFormatHasAlpha(handle->pix_fmt))
	{
		lastError = String{ "Encoder opened without the requested alpha pixel format.", StringEncoding::ASCII };
		CLOG_ERROR("Encoder '{}' opened with non-alpha pixel format '{}' for requested alpha format '{}'.",
			name,
			PixelFormatName(handle->pix_fmt),
			PixelFormatName(CAST(encodeInfo.format)));
		avcodec_free_context(&handle);
		return;
	}

	if (handle->codec_type == AVMEDIA_TYPE_AUDIO && handle->frame_size == 0)
	{
		handle->frame_size = 1024;
	}
}

FFCodec::~FFCodec()
{
	if (device)
	{
		av_buffer_unref(&device);
	}
    if (handle)
    {
        if (handle->extradata)
        {
            av_freep(&handle->extradata);
        }
        avcodec_free_context(&handle);
    }
	if (frame)
	{
		av_frame_free(&frame);
	}
	// Note: previously this was `if (!swrContext)` which is logically inverted —
	// it would only free a *null* handle and would leak any real SwrContext that
	// SetOptions had allocated (a real source of audio resampler leaks during
	// long playback).
	if (swrContext)
	{
		swr_free(&swrContext);
	}

    memoryResource.Reset();
}

CodecError FFCodec::Decode(const CodedFrame &codedFrame)
{
	char err[64];
    int ret = 0;
    auto packet = codedFrame.InterpretAs<AVPacket>();

    if (codedFrame.GetType() == MediaType::Subtitle)
    {
		SubtitleCue cue;
		return DecodeSubtitleCue(codedFrame, cue);
    }
    else
    {
		ret = avcodec_send_packet(handle, packet);
		if (ret == AVERROR(EAGAIN))
		{
			// libavcodec: input buffer full — must drain avcodec_receive_frame first, then resend this packet.
			// (Same contract as ffplay decoder_decode_frame when send_packet returns EAGAIN.)
			return CodecError::Again;
		}
		if (ret < 0)
		{
			if (ret != AVERROR_EOF)
			{
				if (device && ReopenDecoderAsSoftware() == CodecError::Success)
				{
					ret = avcodec_send_packet(handle, packet);
					if (ret == AVERROR(EAGAIN))
					{
						return CodecError::Again;
					}
					if (ret >= 0 || ret == AVERROR_EOF)
					{
						if (packet)
						{
							handle->time_base = packet->time_base;
						}
						return CodecError::Success;
					}
				}
				CLOG_ERROR("Failed to decode frame - {}", AVERR_STR(ret));
				return CodecError::ExternalFailed;
			}
		}

		if (packet)
		{
			handle->time_base = packet->time_base;
		}
    }

    return CodecError::Success;
}

CodecError FFCodec::GetPicture(Picture &picture)
{
    int ret = avcodec_receive_frame(handle, frame);
    if (ret < 0)
    {
        if (ret == AVERROR(EAGAIN))
        {
            return CodecError::Again;
        }
        if (ret == AVERROR_EOF)
        {
            return CodecError::EndOfFile;
        }
		if (device && ReopenDecoderAsSoftware() == CodecError::Success)
		{
			return CodecError::Again;
		}
        return CodecError::ExternalFailed;
    }

    AVRational timeBase = handle->time_base;
    if (handle->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        const AVPixelFormat decodedPixelFormat = (AVPixelFormat)frame->format;
        const AVPixFmtDescriptor *decodedFormatDescriptor = av_pix_fmt_desc_get(decodedPixelFormat);
        const bool hardwareFrame = decodedFormatDescriptor &&
            (decodedFormatDescriptor->flags & AV_PIX_FMT_FLAG_HWACCEL);
        bool directDeviceFrame = false;
#ifdef _WIN32
        directDeviceFrame = decodedPixelFormat == AV_PIX_FMT_D3D12 &&
            Graphics::GetDevice()->GetBackendAPI() == BackendAPI::D3D12;
#endif

        AVFrame *ref = NULL;
        if (hardwareFrame && !directDeviceFrame)
        {
            ref = av_frame_alloc();
            if (!ref)
            {
                return CodecError::OutOfMemory;
            }

            if (type == PictureMemoryType::System)
            {
				ret = av_hwframe_transfer_data(ref, frame, 0);
            }
            else if (type == PictureMemoryType::Device && hwaccelType == AV_HWDEVICE_TYPE_VIDEOTOOLBOX)
            {
                ret = av_hwframe_map(ref, frame, AV_HWFRAME_MAP_READ);
            }
            if (ret < 0)
            {
                CLOG_ERROR("Failed to map frame or download frame to system memory!");
                av_frame_free(&ref);
                av_frame_unref(frame);

				if (device && ReopenDecoderAsSoftware() == CodecError::Success)
				{
					return CodecError::Again;
				}
                return CodecError::ExternalFailed;
            }
        }
        else
        {
            ref = av_frame_clone(frame);
        }

        const AVPixelFormat framePixelFormat = (AVPixelFormat)ref->format;
        // Hardware downloads can expose a planar layout even when the device layout is P010/NV12.
        enum AVPixelFormat pixelFormat = framePixelFormat;
        if (directDeviceFrame)
        {
            pixelFormat = handle->sw_pix_fmt;
            if (device && handle->pix_fmt != AV_PIX_FMT_YUV422P10)
            {
                switch (pixelFormat)
                {
                    case AV_PIX_FMT_YUV444P10:
                    case AV_PIX_FMT_YUV422P10:
                    case AV_PIX_FMT_YUV420P10:
                        pixelFormat = AV_PIX_FMT_P010;
                        break;

                    case AV_PIX_FMT_YUV444P:
                    case AV_PIX_FMT_YUV422P:
                    case AV_PIX_FMT_YUV420P:
                    case AV_PIX_FMT_YUVJ420P:
                        pixelFormat = AV_PIX_FMT_NV12;
                        break;

                    default:
                        break;
                }
            }
            else
            {
                pixelFormat = handle->pix_fmt;
            }
        }
        format = CAST(pixelFormat);

        const bool directFloat32Rgb =
            IsBigEndianFloat32RgbFormat(framePixelFormat) ||
            IsLittleEndianFloat32RgbFormat(framePixelFormat);

		if (format == Format::None && directFloat32Rgb)
		{
			picture = CopyFloat32RgbFrameToRgba32F(ref);
            if (!picture)
            {
                av_frame_unref(ref);
                av_frame_free(&ref);
                av_frame_unref(frame);
                return CodecError::ExternalFailed;
            }
            picture.SetColorSpace(colorSpace);
            if (ref->color_range == AVCOL_RANGE_JPEG)
            {
                picture.SetFlags(PictureFlags::FullRange);
            }
			av_frame_unref(ref);
			av_frame_free(&ref);
		}
		else if (format == Format::None)
		{
			picture = ScaleToSupportFormat(ref);
			if (picture)
			{
				picture.SetColorSpace(colorSpace);
				if (ref->color_range == AVCOL_RANGE_JPEG)
				{
					picture.SetFlags(PictureFlags::FullRange);
				}
			}
			av_frame_unref(ref);
			av_frame_free(&ref);
		}
		else
		{
			picture = Picture{ref->width, ref->height, format};
			picture.SetStride(0, ref->linesize[0]);
			picture.SetStride(1, ref->linesize[1]);
			picture.SetStride(2, ref->linesize[2]);
			picture.SetStride(3, ref->linesize[3]);
			picture.SetColorSpace(colorSpace);
			if (ref->color_range == AVCOL_RANGE_JPEG)
			{
				picture.SetFlags(PictureFlags::FullRange);
			}

#	ifdef _WIN32
			if (directDeviceFrame)
			{
				AVD3D12VAFrame *f = (AVD3D12VAFrame *) frame->data[0];
				auto &[texture, index, syncCtx, flags] = *f;
				picture.SetMemoryType(PictureMemoryType::Device);
				picture[0] = (uint8_t *) texture;
				picture[1] = (uint8_t *) syncCtx.fence;
				picture[2] = (uint8_t *) syncCtx.fence_value;
			}
			else
#	endif
			{
				picture.SetMemoryType(PictureMemoryType::System);

				picture[0] = (uint8_t *) ref->data[0];
				picture[1] = (uint8_t *) ref->data[1];
				picture[2] = (uint8_t *) ref->data[2];
				picture[3] = (uint8_t *) ref->data[3];
			}

			picture.SetRelease([ref](void *) {
				av_frame_unref(ref);
				av_frame_free((AVFrame **) &ref);
			});
		}

		picture.SetSampleAspectRatio(ResolveSampleAspectRatio(frame, handle, sampleAspectRatio));
        picture.SetTimestamp(NAN);
		if (frame->pts != AV_NOPTS_VALUE)
		{
			//picture.SetTimestamp((frame->best_effort_timestamp - startTimestamp) * av_q2d(timeBase) * animator.FramesPerSecond);
			picture.SetTimestamp(frame->best_effort_timestamp - startTimestamp);
			picture.SetTimebase({timeBase.num, timeBase.den});
		}
    }
    else if (handle->codec_type == AVMEDIA_TYPE_AUDIO)
    {
		AVRational tb = handle->time_base;
		AVRational sampleTb = { 1, frame->sample_rate };

		if (frame->pts != AV_NOPTS_VALUE)
		{
			frame->pts = av_rescale_q(frame->pts - startTimestamp, tb, sampleTb);
		}
		else if (audioNextPtsSync != AV_NOPTS_VALUE)
		{
			frame->pts = av_rescale_q(audioNextPtsSync, { (int)audioNextPtsTbNum, (int)audioNextPtsTbDen }, sampleTb);
		}
		if (frame->pts != AV_NOPTS_VALUE)
		{
			audioNextPtsSync  = frame->pts + frame->nb_samples;
			audioNextPtsTbNum = sampleTb.num;
			audioNextPtsTbDen = sampleTb.den;
		}

		AVFrame *ref = av_frame_clone(frame);
        picture = Picture{ frame->nb_samples, frame->ch_layout.nb_channels, CAST(handle->sample_fmt) };
		picture.SetSampleRate(frame->sample_rate);

        for (int i = 0; i < frame->ch_layout.nb_channels; i++)
        {
			picture.SetDataAt(i, frame->data[i]);
			picture.SetStride(i, frame->linesize[i]);
        }

		picture.SetRelease([ref](void *) {
			av_frame_unref(ref);
			av_frame_free((AVFrame **)&ref);
		});

		picture.SetTimebase({sampleTb.num, sampleTb.den});
        picture.SetTimestamp(frame->pts);
    }

    av_frame_unref(frame);

	return CodecError::Success;
}

static std::string TrimSubtitleText(std::string text)
{
	auto isSpace = [](unsigned char c) {
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	};

	size_t begin = 0;
	while (begin < text.size() && isSpace((unsigned char)text[begin]))
	{
		begin++;
	}

	size_t end = text.size();
	while (end > begin && isSpace((unsigned char)text[end - 1]))
	{
		end--;
	}

	return text.substr(begin, end - begin);
}

static std::string DecodeAssSubtitleText(const char *ass)
{
	if (!ass || !*ass)
	{
		return {};
	}

	std::string text{ ass };
	const bool fullDialogueLine = text.rfind("Dialogue:", 0) == 0;
	const size_t fieldSeparatorCount = fullDialogueLine ? 9 : 8;
	size_t textStartAfterFields = std::string::npos;
	size_t commaCount = 0;
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == ',')
		{
			commaCount++;
			if (commaCount == fieldSeparatorCount)
			{
				textStartAfterFields = i + 1;
				break;
			}
		}
	}
	if (textStartAfterFields != std::string::npos && textStartAfterFields < text.size())
	{
		text = text.substr(textStartAfterFields);
	}

	std::string cleaned;
	cleaned.reserve(text.size());
	bool inTag = false;
	for (size_t i = 0; i < text.size(); i++)
	{
		char c = text[i];
		if (c == '{')
		{
			inTag = true;
			continue;
		}
		if (c == '}')
		{
			inTag = false;
			continue;
		}
		if (inTag)
		{
			continue;
		}
		if (c == '\\' && i + 1 < text.size())
		{
			char next = text[i + 1];
			if (next == 'N' || next == 'n')
			{
				cleaned.push_back('\n');
				i++;
				continue;
			}
			if (next == 'h')
			{
				cleaned.push_back(' ');
				i++;
				continue;
			}
		}
		cleaned.push_back(c);
	}

	return TrimSubtitleText(std::move(cleaned));
}

static bool AppendSubtitleBitmapRect(SubtitleCue &cue, const AVSubtitleRect *rect, int canvasWidth, int canvasHeight)
{
	if (!rect || rect->type != SUBTITLE_BITMAP || rect->w <= 0 || rect->h <= 0 || !rect->data[0] || !rect->data[1])
	{
		return false;
	}

	const uint32_t width = (uint32_t)rect->w;
	const uint32_t height = (uint32_t)rect->h;
	const uint32_t stride = width * 4;
	SubtitleCue::BitmapRect bitmap{};
	bitmap.width = width;
	bitmap.height = height;
	bitmap.stride = stride;
	bitmap.rgba.resize((size_t)stride * height);

	const uint8_t *src = rect->data[0];
	const int srcStride = rect->linesize[0] > 0 ? rect->linesize[0] : rect->w;
	const uint32_t *palette = reinterpret_cast<const uint32_t *>(rect->data[1]);
	for (uint32_t y = 0; y < height; y++)
	{
		const uint8_t *srcRow = src + (size_t)y * srcStride;
		uint8_t *dstRow = bitmap.rgba.data() + (size_t)y * stride;
		for (uint32_t x = 0; x < width; x++)
		{
			const uint32_t color = palette[srcRow[x]];
			uint8_t *dst = dstRow + (size_t)x * 4;
			dst[0] = (uint8_t)((color >> 16) & 0xff);
			dst[1] = (uint8_t)((color >>  8) & 0xff);
			dst[2] = (uint8_t)( color        & 0xff);
			dst[3] = (uint8_t)((color >> 24) & 0xff);
		}
	}

	if (canvasWidth <= 0)
	{
		canvasWidth = std::max(rect->x + rect->w, rect->w);
	}
	if (canvasHeight <= 0)
	{
		canvasHeight = std::max(rect->y + rect->h, rect->h);
	}
	if (canvasWidth > 0 && canvasHeight > 0)
	{
		bitmap.x = std::clamp((float)rect->x / (float)canvasWidth, 0.0f, 1.0f);
		bitmap.y = std::clamp((float)rect->y / (float)canvasHeight, 0.0f, 1.0f);
		bitmap.widthRatio = std::clamp((float)rect->w / (float)canvasWidth, 0.0f, 1.0f);
		bitmap.heightRatio = std::clamp((float)rect->h / (float)canvasHeight, 0.0f, 1.0f);
	}
	else
	{
		bitmap.x = 0.0f;
		bitmap.y = 0.0f;
		bitmap.widthRatio = 1.0f;
		bitmap.heightRatio = 1.0f;
	}

	if (bitmap.widthRatio <= 0.0f || bitmap.heightRatio <= 0.0f)
	{
		return false;
	}

	if (!cue.hasRect)
	{
		cue.x = bitmap.x;
		cue.y = bitmap.y;
		cue.width = bitmap.widthRatio;
		cue.height = bitmap.heightRatio;
		cue.hasRect = true;
	}
	cue.bitmaps.emplace_back(std::move(bitmap));
	return true;
}

static bool IsValidTimeBase(const AVRational &timeBase)
{
	return timeBase.num > 0 && timeBase.den > 0;
}

static AVRational ResolveSubtitlePacketTimeBase(const AVPacket *packet, const AVCodecContext *context)
{
	if (packet && IsValidTimeBase(packet->time_base))
	{
		return packet->time_base;
	}
	if (context && IsValidTimeBase(context->pkt_timebase))
	{
		return context->pkt_timebase;
	}
	if (context && IsValidTimeBase(context->time_base))
	{
		return context->time_base;
	}
	return { 1, AV_TIME_BASE };
}

static double SubtitlePacketDurationSeconds(const AVPacket *packet, const AVRational &timeBase)
{
	if (!packet || packet->duration <= 0 || !IsValidTimeBase(timeBase))
	{
		return 0.0;
	}
	return (double)packet->duration * av_q2d(timeBase);
}

static void AppendSubtitleLine(std::string &dst, std::string &&line)
{
	line = TrimSubtitleText(std::move(line));
	if (line.empty())
	{
		return;
	}
	if (!dst.empty())
	{
		dst.push_back('\n');
	}
	dst += line;
}

CodecError FFCodec::DecodeSubtitleCue(const CodedFrame &codedFrame, SubtitleCue &cue)
{
	cue = {};
	if (!handle || handle->codec_type != AVMEDIA_TYPE_SUBTITLE)
	{
		return CodecError::InvalidArguments;
	}
	if (!codedFrame)
	{
		return CodecError::Again;
	}

	AVPacket *packet = codedFrame.InterpretAs<AVPacket>();
	if (!packet)
	{
		return CodecError::InvalidArguments;
	}

	int got = 0;
	AVSubtitle subtitle{};
	int ret = avcodec_decode_subtitle2(handle, &subtitle, &got, packet);
	if (ret < 0)
	{
		char err[64] = {};
		CLOG_ERROR("Failed to decode subtitle - {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}
	if (!got)
	{
		return CodecError::Again;
	}

	std::string text;
	for (uint32_t i = 0; i < subtitle.num_rects; i++)
	{
		AVSubtitleRect *rect = subtitle.rects[i];
		if (!rect)
		{
			continue;
		}
		if (rect->ass && rect->ass[0])
		{
			AppendSubtitleLine(text, DecodeAssSubtitleText(rect->ass));
		}
		else if (rect->text && rect->text[0])
		{
			AppendSubtitleLine(text, std::string{ rect->text });
		}
		else if (rect->type == SUBTITLE_BITMAP)
		{
			AppendSubtitleBitmapRect(cue, rect, handle->width, handle->height);
		}

		if (!cue.hasRect && rect->w > 0 && rect->h > 0 && handle->width > 0 && handle->height > 0)
		{
			cue.x = std::clamp((float)rect->x / (float)handle->width, 0.0f, 1.0f);
			cue.y = std::clamp((float)rect->y / (float)handle->height, 0.0f, 1.0f);
			cue.width = std::clamp((float)rect->w / (float)handle->width, 0.0f, 1.0f);
			cue.height = std::clamp((float)rect->h / (float)handle->height, 0.0f, 1.0f);
			cue.hasRect = true;
		}
	}

	const AVRational packetTimeBase = ResolveSubtitlePacketTimeBase(packet, handle);
	double baseSeconds = 0.0;
	if (subtitle.pts != AV_NOPTS_VALUE)
	{
		int64_t subtitlePts = subtitle.pts;
		if (startTimestamp != AV_NOPTS_VALUE)
		{
			if (IsValidTimeBase(packetTimeBase))
			{
				subtitlePts -= av_rescale_q(startTimestamp, packetTimeBase, AV_TIME_BASE_Q);
			}
		}
		baseSeconds = (double)subtitlePts / (double)AV_TIME_BASE;
	}
	else
	{
		int64_t pts = packet->pts;
		if (pts == AV_NOPTS_VALUE)
		{
			pts = packet->dts;
		}
		if (pts == AV_NOPTS_VALUE)
		{
			pts = codedFrame.GetTimestamp();
		}
		if (pts == AV_NOPTS_VALUE)
		{
			pts = 0;
		}
		if (startTimestamp != AV_NOPTS_VALUE)
		{
			pts -= startTimestamp;
		}

		baseSeconds = (double)pts * av_q2d(packetTimeBase);
	}

	static constexpr double DefaultSubtitleDurationSeconds = 4.0;
	static constexpr double MaxReasonableSubtitleOffsetSeconds = 300.0;
	static constexpr double MaxReasonableSubtitleDurationSeconds = 300.0;
	double startOffsetSeconds = (double)subtitle.start_display_time / 1000.0;
	if (!std::isfinite(startOffsetSeconds) ||
	    startOffsetSeconds < 0.0 ||
	    startOffsetSeconds > MaxReasonableSubtitleOffsetSeconds)
	{
		startOffsetSeconds = 0.0;
	}

	double durationSeconds = ((double)subtitle.end_display_time - (double)subtitle.start_display_time) / 1000.0;
	if (!std::isfinite(durationSeconds) ||
	    durationSeconds <= 0.0 ||
	    durationSeconds > MaxReasonableSubtitleDurationSeconds)
	{
		const double packetDuration = SubtitlePacketDurationSeconds(packet, packetTimeBase);
		durationSeconds = std::isfinite(packetDuration) &&
			packetDuration > 0.0 &&
			packetDuration <= MaxReasonableSubtitleDurationSeconds ?
			packetDuration :
			DefaultSubtitleDurationSeconds;
	}

	cue.startSeconds = baseSeconds + startOffsetSeconds;
	cue.endSeconds = cue.startSeconds + durationSeconds;
	cue.text = TrimSubtitleText(std::move(text));

	avsubtitle_free(&subtitle);
	return cue ? CodecError::Success : CodecError::Again;
}

void ReleasePicture(void *opaque, uint8_t *data)
{
    {
		Ref<IObject> object;
		object.Attach(opaque);
    }
}

class AVFrameWrapper
{
public:
	AVFrameWrapper() :
	    frame{}
	{
		frame = av_frame_alloc();
	}

	~AVFrameWrapper()
	{
		if (frame)
		{
			av_frame_free(&frame);
			frame = nullptr;
		}
	}

    operator AVFrame *() const
    {
		return frame;
    }

	AVFrame *frame;
};

class AVPacketWrapper
{
public:
	AVPacketWrapper() :
	    packet{}
	{
		packet = av_packet_alloc();
	}

	~AVPacketWrapper()
	{
		if (packet)
		{
			av_packet_free(&packet);
			packet = nullptr;
		}
	}

    AVPacket *Detach()
    {
		AVPacket *ret = nullptr;
		std::swap(ret, packet);
		return ret;
    }

	operator AVPacket *() const
	{
		return packet;
	}

	AVPacket *packet;
};

CodecError FFCodec::RescaleAudioSamples(int numOutSamples, uint8_t *const *out, int outSampleRate,int numInSamples, const uint8_t *const *in, int inSampleRate, int dataSize)
{
	int outSamples = swr_convert(swrContext, out, numOutSamples, in, numInSamples);
	if (outSamples < 0)
	{
		CLOG_ERROR("Failed to rescale audio frame format!");
		return CodecError::ExternalFailed;
	}

	uint8_t *ptr = out[0] + outSamples * dataSize;
	//if (swr_get_out_samples(swrContext, 0) > 0)
	//{
	//	outSamples = swr_convert(swrContext, &ptr, numOutSamples - outSamples, nullptr, 0);
	//}

    return CodecError::Success;
}


CodecError FFCodec::EncodeFrame(AVFrame *frame)
{
	char err[64] = {};
    int ret = av_frame_make_writable(frame);
    if (ret < 0)
    {
		lastError = FFmpegErrorText("Error when making encoder frame writable: ", ret);
		CLOG_ERROR("Error when making frame writable - {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
    }

	ret = avcodec_send_frame(handle, frame);
	if (ret == AVERROR(EAGAIN))
	{
		return CodecError::Again;
	}
	if (ret == AVERROR_EOF)
	{
		return CodecError::EndOfFile;
	}
	if (ret < 0)
	{
		lastError = FFmpegErrorText("Error sending frame to encoder: ", ret);
		CLOG_ERROR("Error sending frame to encoder: {}", AVERR_STR(ret));
		return CodecError::ExternalFailed;
	}

    return CodecError::Success;
}

Picture FFCodec::ScaleToSupportFormat(AVFrame *frame)
{
	Picture picture{};
	AVPixelFormat dstFormat = frame->format == AV_PIX_FMT_XV36 ? AV_PIX_FMT_YUV444P10 : AV_PIX_FMT_NV12;
	picture = Picture{frame->width, frame->height, CAST(dstFormat), true};
	scaler.Init(frame->width, frame->height, frame->format, picture.GetWidth(), picture.GetHeight(), (int)dstFormat, SWS_BILINEAR);

	CodecError ret = scaler.Scale(frame->data, frame->linesize, 0, frame->height, &picture.GetData(), (const int *) &picture.GetStride());
	if (ret != CodecError::Success)
	{
		return {};
	}

	return picture;
}

CodecError FFCodec::SendAudioFifo()
{
	char err[64] = {};
    while (fifo.Size() >= handle->frame_size)
    {
		AVFrameWrapper wrapper{};
		AVFrame *frame = wrapper;

		int frameSize = std::min(fifo.Size(), handle->frame_size);
		frame->nb_samples  = frameSize;
        frame->format      = handle->sample_fmt;
		frame->sample_rate = handle->sample_rate;
		av_channel_layout_copy(&frame->ch_layout, &handle->ch_layout);

        int ret = av_frame_get_buffer(frame, 0);
		if (ret < 0)
        {
			CLOG_ERROR("Failed to allocate buffer for frame samples - {}", AVERR_STR(ret));
			return CodecError::OutOfMemory;
        }

        if (fifo.Read((void **)frame->data, frameSize) < frameSize)
		{
			CLOG_ERROR("Could not read data from FIFO");
			return CodecError::ExternalFailed;
		}

        frame->pts = pts;
		pts += frame->nb_samples;
		frame->pts = av_rescale_q(frame->pts, AVRational{ 1, handle->sample_rate }, handle->time_base);

		auto error = EncodeFrame(frame);
        if (error != CodecError::Success)
        {
			return error;
        }
    }

    return CodecError::Success;
}

void FreeIcc(void *opaque, uint8_t *data)
{
	std::vector<uint8_t> *icc = (std::vector<uint8_t> *) opaque;
	delete icc;
}

CodecError FFCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	char err[64] = {};
	int ret;

    if (handle->codec_type == AVMEDIA_TYPE_VIDEO)
    {
		Picture ref = picture;
		auto format = CAST(picture.GetFormat());
        if (format != handle->pix_fmt ||
			picture.GetWidth()  != handle->width ||
			picture.GetHeight() != handle->height)
        {
            if (!scaler)
            {
				scaler.Init(picture.GetWidth(), picture.GetHeight(), picture.GetFormat(), handle->width, handle->height, CAST(handle->pix_fmt), Filter::Bilinear);
                this->picture = Picture{ handle->width, handle->height, CAST(handle->pix_fmt), true };
            }

			CodecError ret = scaler.Scale(this->picture, picture);
			if (ret != CodecError::Success)
			{
				CLOG_ERROR("Failed to scale picture");
			}
			ref = this->picture;
        }
        else if (picture.GetWidth() != handle->width || picture.GetHeight() != handle->height)
        {
			if (!this->picture)
			{
				this->picture = Picture{handle->width, handle->height, CAST(handle->pix_fmt), true};
			}

            BicubicConvolutionInterpolate(this->picture, picture);
            ref = this->picture;
        }

		auto wrapper = AVFrameWrapper();
		AVFrame *frame = wrapper;
		if (!frame)
		{
			CLOG_ERROR("Failed to allocate frame!");
			return CodecError::OutOfMemory;
		}

		auto object = picture.GetIObject();
		object->AddRef();
		AVBufferRef *buf = av_buffer_create((uint8_t *)&object, sizeof(object), ReleasePicture, object, 0);
		if (!buf)
		{
			CLOG_ERROR("Failed to allocate buffer ref");
			return CodecError::OutOfMemory;
		}
		frame->buf[0] = buf;

		for (int i = 0; ref.GetData(i); i++)
		{
			frame->data[i]     = ref.GetData(i);
			frame->linesize[i] = ref.GetStride(i);
		}

		frame->pts    = picture.GetTimestamp();
		frame->format = handle->pix_fmt;
		frame->width  = ref.GetWidth();
		frame->height = ref.GetHeight();
        if (IsValidSampleAspectRatio(picture.GetSampleAspectRatio()))
        {
			frame->sample_aspect_ratio = AVRational{ (int)picture.GetSampleAspectRatio().numerator, (int)picture.GetSampleAspectRatio().denominator };
        }
		const Rational &pictureTimebase = picture.GetTimebase();
		if (pictureTimebase.numerator != 0 && pictureTimebase.denominator != 0)
		{
			frame->pts = av_rescale_q(
				frame->pts,
				AVRational{ (int)pictureTimebase.numerator, (int)pictureTimebase.denominator },
				handle->time_base);
		}
		else
		{
			frame->pts = av_rescale_q(frame->pts, av_inv_q(handle->framerate), handle->time_base);
		}

		auto icc = picture.GetProperty<ICCProfileProperty>();
		if (icc)
		{
			std::vector<uint8_t> *profile = new std::vector<uint8_t>;
			*profile = icc->profile;
			AVBufferRef *buf = av_buffer_create(profile->data(), profile->size(), FreeIcc, profile, 0);
			av_frame_side_data_add(&frame->side_data, &frame->nb_side_data, AV_FRAME_DATA_ICC_PROFILE, &buf, 0);
			av_buffer_unref(&buf);
		}

        return EncodeFrame(frame);
	}
	else if (handle->codec_type == AVMEDIA_TYPE_AUDIO)
	{
        if (!fifo)
        {
			if (fifo.Allocate(handle->sample_fmt, handle->ch_layout.nb_channels, handle->frame_size ? handle->frame_size : 1024) < 0)
			{
				return CodecError::OutOfMemory;
			}
        }
        AVSampleFormat sampleFormat = CastSampleFormat(picture.GetFormat());
        if (sampleFormat != handle->sample_fmt ||
            picture.GetHeight() != handle->ch_layout.nb_channels ||
            picture.GetSampleRate() != handle->sample_rate)
        {
            if (!swrContext)
            {
				AVChannelLayout intputLayout;
				av_channel_layout_default(&intputLayout, picture.GetHeight());
				ret = swr_alloc_set_opts2(
				    &swrContext,
				    &handle->ch_layout,
				    handle->sample_fmt,
				    handle->sample_rate,
				    &intputLayout,
				    sampleFormat,
				    picture.GetSampleRate(),
				    0,
				    nullptr);

			    ret = swr_init(swrContext);
				if (ret < 0)
				{
					CLOG_ERROR("Failed to init swr - {}", AVERR_STR(ret));
					return CodecError::ExternalFailed;
				}

                if (ret < 0)
				{
					CLOG_ERROR("Failed to allocate SwrContext");
					return CodecError::OutOfMemory;
				}
            }

            int intputFrameSize = picture.GetWidth();
            int numSamples = av_rescale_rnd(
			    swr_get_delay(swrContext, picture.GetSampleRate()) + picture.GetWidth(),
			    handle->sample_rate,
			    picture.GetSampleRate(),
			    AV_ROUND_UP);

            ret = 0;
            if (numSamples >= numRescaledSamples)
            {
				numRescaledSamples = numSamples;
				rescaledSamples = nullptr;
            }

			if (!rescaledSamples)
			{
				ret = av_samples_alloc_array_and_samples(
				    &rescaledSamples,
				    nullptr,
				    handle->ch_layout.nb_channels,
				    numRescaledSamples,
				    handle->sample_fmt,
				    0);
				if (ret < 0)
				{
					CLOG_ERROR("Failed to allocate array and samples - {}", AVERR_STR(ret));
					return CodecError::OutOfMemory;
				}
			}

			ret = swr_convert(swrContext, rescaledSamples, numRescaledSamples, &picture.GetData(), intputFrameSize);
            if (ret < 0)
            {
				CLOG_ERROR("Could not convert input samples - {}", AVERR_STR(ret));
				return CodecError::ExternalFailed;
            }

            fifo.Enqueue(rescaledSamples, ret);
        }
        else
        {
			fifo.Enqueue(&picture.GetData(), picture.GetWidth());
        }

        return SendAudioFifo();
	}

	return CodecError::InvalidArguments;
}

CodedFrame FFCodec::GetCodedFrame() const
{
	char err[64];

    AVPacketWrapper packetWrapper;
	AVPacket *packet = packetWrapper;
	if (!packet)
	{
		lastError = String{ "Failed to allocate encoder packet.", StringEncoding::ASCII };
		CLOG_ERROR("Failed to alloc packet");
		return {};
	}

	int ret = avcodec_receive_packet(handle, packet);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		return {};
	}
	else if (ret < 0)
	{
		lastError = FFmpegErrorText("Error encoding a frame: ", ret);
		CLOG_ERROR("Error encoding a frame: {}", AVERR_STR(ret));
		return {};
	}

	packet->time_base = handle->time_base;

    packet = packetWrapper.Detach();
	CodedFrame codedFrame = {packet->data, (size_t)packet->size};
	codedFrame.SetAnonymous(packet);

	codedFrame.SetType(mediaType);
	codedFrame.SetTimestamp(packet->pts);
	codedFrame.SetRelease([packet](void *data) {
		av_packet_free((AVPacket **)&packet);
	});

    return codedFrame;
}

void FFCodec::FlushAudioFifo()
{
    char err[64]{};
	int ret = swr_convert(swrContext, rescaledSamples, numRescaledSamples, nullptr, 0);
	if (ret < 0)
	{
		CLOG_ERROR("Could not convert input samples - {}", AVERR_STR(ret));
		avcodec_send_frame(handle, nullptr);
		return;
	}

	fifo.Enqueue(rescaledSamples, ret);
	auto size = fifo.Size();
	ret = swr_inject_silence(swrContext, handle->frame_size - size);
    if (ret < 0)
    {
		CLOG_ERROR("Could not inject silence - {}", AVERR_STR(ret));
		avcodec_send_frame(handle, nullptr);
		return;
    }

	ret = swr_convert(swrContext, rescaledSamples, numRescaledSamples, nullptr, 0);
	if (ret < 0)
	{
		CLOG_ERROR("Could not convert input samples - {}", AVERR_STR(ret));
		avcodec_send_frame(handle, nullptr);
		return;
	}
	fifo.Enqueue(rescaledSamples, ret);

    SendAudioFifo();
}

void FFCodec::Flush()
{
	if (handle)
	{
        if (isEncoder)
        {
			if (handle->codec_type == AVMEDIA_TYPE_AUDIO && swrContext)
            {
				FlushAudioFifo();
            }
		    avcodec_send_frame(handle, nullptr);
        }
        else
        {
			avcodec_flush_buffers(handle);
			if (handle->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audioNextPtsSync     = AV_NOPTS_VALUE;
				audioNextPtsTbNum    = 0;
				audioNextPtsTbDen    = 1;
			}
			//avcodec_send_packet(handle, nullptr);
        }
	}
}

void *FFCodec::GetProperty(PropertyType type) const
{
	switch (type)
	{
		case PropertyType::DisplayOrientation:
			return (void *)&displayOrientation;
		default:
			return nullptr;
	}
}

AVHWDeviceType GetDeviceType(const std::string &name)
{
    if (name.find("qsv") != std::string::npos)
    {
        return AV_HWDEVICE_TYPE_QSV;
    }
    if (name.find("cuvid") != std::string::npos || name.find("nvdec") != std::string::npos)
    {
        return AV_HWDEVICE_TYPE_CUDA;
    }

    return AV_HWDEVICE_TYPE_NONE;
}

static std::list<const char *> QueryDecoderPriorities(const AVCodecID id)
{
    switch (id)
    {
    case AV_CODEC_ID_H264:
        return {
            "h264_cuvid",
            "h264_qsv",
        };

    case AV_CODEC_ID_HEVC:
        return {
            "hevc_qsv",
            "hevc_cuvid",
        };

    case AV_CODEC_ID_AV1:
        return {
            "av1_qsv",
            "libdav1d",
            "av1_nvdec",
        };

    case AV_CODEC_ID_MJPEG:
        return {
            "mjpeg_nvdec",
            "mjpeg_qsv",
        };

    case AV_CODEC_ID_MPEG4:
        return {
            "mpeg4_nvdec"
        };

    case AV_CODEC_ID_MPEG1VIDEO:
        return {
            "mpeg1_nvdec"
        };

    case AV_CODEC_ID_MPEG2VIDEO:
        return {
            "mpeg2_nvdec",
            "mpeg2_qsv",
        };

    case AV_CODEC_ID_VC1:
        return {
            "vc1_nvdec"
        };

    case AV_CODEC_ID_WMV3:
        return {
            "wmv3_nvdec"
        };

    case AV_CODEC_ID_VP8:
        return {
            "vp8_nvdec"
        };

    case AV_CODEC_ID_VP9:
        return {
            "vp9_nvdec",
            "vp9_qsv",
        };

    default:
        return {};
    }
}

static AVHWDeviceType QueryDecoderHWAccelType()
{
    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;

    static std::list<const char *> priorities = {
#ifdef __APPLE__
        "videotoolbox",
#endif
        "d3d12va",
    };

    for (auto &p : priorities)
    {
        type = av_hwdevice_find_type_by_name(p);
        if (type != AV_HWDEVICE_TYPE_NONE)
        {
            break;
        }
    }

    return type;
}

CodecError FFCodec::CreateHardwareAccelerateDevice(const AVCodec *codec)
{
    hwaccelType = QueryDecoderHWAccelType();
    for (int i = 0; ; i++)
    {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if (!config)
        {
            break;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == hwaccelType)
        {
#ifdef _WIN32
            if (hwaccelType == AV_HWDEVICE_TYPE_D3D12VA && Graphics::GetDevice()->GetBackendAPI() == BackendAPI::D3D12)
            {
				device = av_hwdevice_ctx_alloc((AVHWDeviceType)hwaccelType);
                if (!device)
                {
                    return CodecError::OutOfMemory;
                }

                AVHWDeviceContext *deviceContext  = (AVHWDeviceContext *)device->data;
                AVD3D12VADeviceContext *d3d12vaContext = (AVD3D12VADeviceContext *)deviceContext->hwctx;

                d3d12vaContext->device = (ID3D12Device *)Graphics::GetDevice()->GetBackendHandle();
                d3d12vaContext->device->AddRef();
                av_hwdevice_ctx_init(device);

                type = PictureMemoryType::Device;
            }
            else
#endif
            if (av_hwdevice_ctx_create(&device, (AVHWDeviceType)hwaccelType, NULL, NULL, 0) < 0)
            {
                CLOG_ERROR("Failed to create specified HW device.");
                return CodecError::NotImplement;
            }
#ifdef __APPLE__
            if (hwaccelType == AV_HWDEVICE_TYPE_VIDEOTOOLBOX && Graphics::GetDevice()->GetBackendAPI() == BackendAPI::Metal)
            {
                type = PictureMemoryType::Device;
            }
#endif
            break;
        }
    }

    return CodecError::Success;
}

CodecError FFCodec::ReopenDecoderAsSoftware()
{
	if (hardwareFallbackAttempted || !decoderStream || !handle || handle->codec_type != AVMEDIA_TYPE_VIDEO)
	{
		return CodecError::ExternalFailed;
	}

	hardwareFallbackAttempted = true;
	CLOG_WARN("Hardware decoding failed; reopening decoder in software.");

	if (handle)
	{
		if (handle->extradata)
		{
			av_freep(&handle->extradata);
		}
		avcodec_free_context(&handle);
	}
	if (device)
	{
		av_buffer_unref(&device);
	}
	if (frame)
	{
		av_frame_unref(frame);
	}

	type = PictureMemoryType::System;
	format = Format::None;
	hwaccelType = AV_HWDEVICE_TYPE_NONE;
	colorSpace = ColorSpace::BT709;
	transferCharacteristic = ColorTransferCharacteristic::Unspecified;
	preference = DecodingPreference::Software;

	return InitializeDecoder(decoderCodecId, decoderStream);
}

static bool RequiresSoftwareDecoding(AVCodecID codecId, int profile)
{
    if (codecId == AV_CODEC_ID_HEVC)
    {
        return profile == AV_PROFILE_HEVC_REXT;
    }
    if (codecId != AV_CODEC_ID_H264)
    {
        return false;
    }

    switch (profile)
    {
        case AV_PROFILE_H264_HIGH_10:
        case AV_PROFILE_H264_HIGH_10_INTRA:
        case AV_PROFILE_H264_HIGH_422:
        case AV_PROFILE_H264_HIGH_422_INTRA:
        case AV_PROFILE_H264_HIGH_444:
        case AV_PROFILE_H264_HIGH_444_PREDICTIVE:
        case AV_PROFILE_H264_HIGH_444_INTRA:
        case AV_PROFILE_H264_CAVLC_444:
            return true;

        default:
            return false;
    }
}

CodecError FFCodec::InitializeDecoder(int _codecId, const AVStream *stream)
{
    AVCodecID codecId = (AVCodecID)_codecId;
	decoderCodecId = _codecId;
	decoderStream = stream;
    const AVCodec *codec = avcodec_find_decoder(codecId);

	if (codecId == AV_CODEC_ID_PNG)
	{
		codec = avcodec_find_decoder_by_name("png");
	}
    if (!codec)
    {
        return CodecError::NotImplement;
    }

    CodecError error = {};
    const bool requiresSoftwareDecoding = stream &&
        stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
        RequiresSoftwareDecoding(codecId, stream->codecpar->profile);
    if (preference != DecodingPreference::Software && stream && !requiresSoftwareDecoding)
	{
        error = CreateHardwareAccelerateDevice(codec);
        if (error != CodecError::Success)
        {
			return error;
        }

        if (!device && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			hwaccelType = AV_HWDEVICE_TYPE_NONE;
            auto priorities = QueryDecoderPriorities(codecId);
            for (auto p : priorities)
            {
                const AVCodec *externalCodec = avcodec_find_decoder_by_name(p);
                if (externalCodec)
				{
					AVHWDeviceType type = GetDeviceType(p);
					if (type != AV_HWDEVICE_TYPE_NONE)
                    {
						if (av_hwdevice_ctx_create(&device, type, "auto", NULL, 0) < 0)
                        {
                            CLOG_ERROR("Cannot open the hardware device");
                            continue;
                        }
                    }
                    codec = externalCodec;
                    break;
                }
            }
        }
    }

    handle = avcodec_alloc_context3(codec);
    if (!handle)
    {
        CLOG_ERROR("Failed to allocate decode context!");
        return CodecError::OutOfMemory;
    }

    if (device)
    {
        handle->hw_device_ctx = av_buffer_ref(device);
    }

    if (stream)
    {
        if (stream->codecpar->extradata_size)
        {
            handle->extradata = (uint8_t *) av_mallocz(stream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!handle->extradata)
            {
                CLOG_ERROR("Failed to allocate extra data!");
                return CodecError::OutOfMemory;
            }
            memcpy(handle->extradata, stream->codecpar->extradata, stream->codecpar->extradata_size);
            handle->extradata_size = stream->codecpar->extradata_size;
        }

        const AVPacketSideData *packageSideData = av_packet_side_data_get(
            stream->codecpar->coded_side_data,
		    stream->codecpar->nb_coded_side_data,
		    AV_PKT_DATA_DISPLAYMATRIX);
        if (packageSideData)
        {
			double theta = GetDisplayRotation((const int32_t *)packageSideData->data);
			if (fabs(theta - 90) < 1.0)
			{
				displayOrientation.anticlockwiseRotation = -270.0f;
			}
			else if (fabs(theta - 180) < 1.0)
			{
				displayOrientation.anticlockwiseRotation = -180.0f;
			}
			else if (fabs(theta - 270) < 1.0)
			{
				displayOrientation.anticlockwiseRotation = -90.0f;
			}
			else if (fabs(theta) > 1.0)
			{
				CLOG_WARN("Unsupported rotation theta - `{}`", theta);
			}
        }

        int ret = avcodec_parameters_to_context(handle, stream->codecpar);
        if (ret < 0)
        {
            CLOG_ERROR("FFCodec::Failed to fill AVCodecContext parameters");
        }

        sampleAspectRatio = ToRational(stream->codecpar->sample_aspect_ratio);
        handle->pkt_timebase = stream->time_base;
    }

    AVDictionary **opts = (AVDictionary**)av_calloc(1, sizeof(*opts));
	av_dict_set(opts, "threads", device ? "1" : "16", 0);

	//handle->skip_frame = AVDISCARD_NONKEY;
    handle->hwaccel_flags         |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;
    handle->strict_std_compliance |= FF_COMPLIANCE_EXPERIMENTAL;
    if (avcodec_open2(handle, codec, opts) < 0)
    {
        CLOG_ERROR("FFCodec::Failed to open AVCodecContext");
		av_dict_free(opts);
		return CodecError::ExternalFailed;
    }

    av_dict_free(opts);

	colorSpace = ColorSpaceConverter(handle->colorspace);

    return CodecError::Success;
}

CodecError FFCodec::OpenDecoder(CodecInfo &info)
{
	const AVStream *stream = (const AVStream *)info.handle;
	if (stream->start_time != AV_NOPTS_VALUE)
	{
		startTimestamp = stream->start_time;
	}
	const CodecError r = InitializeDecoder(stream->codecpar->codec_id, stream);
	if (r == CodecError::Success && handle && handle->codec_type == AVMEDIA_TYPE_VIDEO && IsValidSampleAspectRatio(info.sampleAspectRatio))
	{
		sampleAspectRatio = info.sampleAspectRatio;
		handle->sample_aspect_ratio = AVRational{ (int)sampleAspectRatio.numerator, (int)sampleAspectRatio.denominator };
	}
	if (r == CodecError::Success && handle && handle->codec_type == AVMEDIA_TYPE_AUDIO)
	{
		audioNextPtsSync  = AV_NOPTS_VALUE;
		audioNextPtsTbNum = 0;
		audioNextPtsTbDen = 1;
	}
	return r;
}

Rational FFCodec::GetFramerate() const
{
	return { handle->framerate.den, handle->framerate.num };
}

Rational FFCodec::GetTimebase() const
{
    return { handle->time_base.num, handle->time_base.den };
}

AudioFormatSpec FFCodec::GetAudioFormat() const
{
	AVChannelLayout layout{};
    AudioFormatSpec spec{};
    if (!handle || handle->codec_type != AVMEDIA_TYPE_AUDIO)
    {
        return spec;
    }

    spec.sampleRate = handle->sample_rate;
    spec.format     = CAST(handle->sample_fmt);
	spec.numChannel = handle->ch_layout.nb_channels;
	if (handle->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
	{
		switch (handle->ch_layout.nb_channels)
		{
			case 1:
				spec.layout = ChannelLayout::Mono;
				break;

			case 2:
			default:
				spec.layout = ChannelLayout::Stereo;
		}
	}
	else
	{
		spec.layout = CAST(handle->ch_layout);
	}

    return spec;
}

FFImageCodec::FFImageCodec(CodecId codecId) :
    Super{},
    codecId{ codecId }
{

}

FFImageCodec::~FFImageCodec()
{
}

CodecError FFImageCodec::Decode(const CodedFrame &codedFrame)
{
	FFCodec codec{codecId};

	AVPacket *packet = av_packet_alloc();
	if (!packet)
	{
		return CodecError::OutOfMemory;
	}
	packet->data = (uint8_t *) codedFrame.GetData();
	packet->size = codedFrame.GetSize();
	packet->pts = 0;

	CodedFrame f{packet};

	CodecError err = codec.Decode(f);
	// Always free the packet — previously the early-return-on-error path
	// leaked the AVPacket (and is hot during JPEG thumbnail decode of large
	// asset libraries).
	av_packet_free(&packet);
	if (err != CodecError::Success)
	{
		return err;
	}
	avcodec_send_packet(codec.GetHandle(), nullptr);

	return codec.GetPicture(picture);
}

CodecError FFImageCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	CodecInfo info{
	    .mediaType = MediaType::Video,
	    .codecId   = codecId,
	    .width     = picture.GetWidth(),
	    .height    = picture.GetHeight(),
	    .format    = picture.GetFormat(),
	    .timeBase  = {1, 1}};

	FFCodec codec{info};

	CodecError err = codec.Encode(picture, codedFrame);
	if (err != CodecError::Success)
	{
		return err;
	}

	avcodec_send_frame(codec.GetHandle(), nullptr);
	codedFrame = codec.GetCodedFrame();
	if (codedFrame)
	{
		return CodecError::Success;
	}

	return CodecError::ExternalFailed;
}

}
}
#endif
