#pragma once

#include "Format.h"
#include "Math/Math.h"

#include <string>
#include <vector>
#include <cstdint>
#include <atomic>

namespace Immortal
{
namespace Vision
{

enum class PictureFlags
{
	Eof       = BIT(0),
	FullRange = BIT(1),
};
SL_ENABLE_BITWISE_OPERATOR(PictureFlags)

enum class ChannelLayout
{
	Mono,
	Stereo,
	_2Point1,
	_2_1,
	Surround,
	_3Point1,
	_4Point0,
	_4Point1,
	_2_2,
	Quad,
	_5Point0,
	_5Point1,
	_5Point0Back,
	_5Point1Back,
	_6Point0,
	_6Point0Front,
	_3Point1Point2,
	Hexagonal,
	_6Point1,
	_6Point1Back,
	_6Point1Front,
	_7Point0,
	_7Point0Front,
	_7Point1,
	_7Point1Wide,
	_7Point1WideBack,
	_5Point1Point2Back,
	Octagonal,
	Cube,
	_5Point1Point4Back,
	_7Point1Point2,
	_7Point1Point4Back,
	_7Point2Point3,
	_9Point1Point4Back,
	Hexadecagonal,
	StereoDownmix,
	_22Point2,
};

struct AudioFormatSpec
{
	Format format;
	ChannelLayout layout;
	int sampleRate;
	int numChannel;
};

enum class PropertyType
{
	Unspecified,
	DisplayOrientation,
	ICCProfile,
};

struct BaseProperty
{
	PropertyType type;
};

struct DisplayOrientation : public BaseProperty
{
	static constexpr PropertyType Type = PropertyType::DisplayOrientation;
	int hflip;
	int anticlockwiseRotation;
};

struct ICCProfileProperty : public BaseProperty
{
	static constexpr PropertyType Type = PropertyType::ICCProfile;
	std::string description;
	std::vector<uint8_t> profile;
};

template <class T>
struct PropertyeTypeGetter
{
	static constexpr PropertyType type = PropertyType::Unspecified;
};

#define DEFINE_PROPERTY_GETTER(T, U)                          \
	template <>                                               \
	struct PropertyeTypeGetter<T>                             \
	{                                                         \
		static constexpr PropertyType type = PropertyType::U; \
	};
DEFINE_PROPERTY_GETTER(DisplayOrientation, DisplayOrientation)
DEFINE_PROPERTY_GETTER(ICCProfileProperty, ICCProfile)

struct TrackInfo
{
	std::string name;
	int streamIndex;
	void *handle;
};

enum class DecodingPreference
{
	Auto,
	Software,
	Hardware,
};

}

enum class MediaType
{
	Video = 0,
	Audio = 1,
	Data = 2,
	Subtitle = 3
};

using PictureFlags = Vision::PictureFlags;

enum class ColorSpace
{
	Unspecified,
	SRGB,
	BT709,
	BT601,
	BT2020,
	S_GAMUT,
	S_GAMUT3,
	S_GAMUT3_CINE,
	DJI_D_GAMUT,
	APPLE_RGB,
	CANON_CINEMA_GAMUT,
	DCI_P3,
	DCI_P3_P,
	P3_D65,
	DISPLAY_P3,
	F_GAMUT,
	F_GAMUT_C,
	V_GAMUT,
	N_GAMUT,
	RED_WIDE_GAMUT_RGB,
	BLACKMAGIC_WIDE_GAMUT,
	PROTUNE_NATIVE,
	ARRI_WIDE_GAMUT_3,
	ARRI_WIDE_GAMUT_4,
	ACES2065_1,
	ACESCG,
	ACESCC,
	ACESCCT,
	ACESPROXY,
	ADOBE_RGB1998,
	ADOBE_WIDE_GAMUT_RGB,
	BETA_RGB,
	CIE_RGB,
	COLOR_MATCH_RGB,
	DAVINCI_WIDE_GAMUT,
	DCDM_XYZ,
	NUM,
};

enum class ColorTransferCharacteristic
{
	Reserved0 = 0,
	BT709 = 1,
	Unspecified = 2,
	Reserved = 3,
	GAMMA22 = 4,
	GAMMA28 = 5,
	SMPTE170M = 6,
	SMPTE240M = 7,
	LINEAR = 8,
	LOG = 9,
	LOG_SQRT = 10,
	IEC61966_2_4 = 11,
	BT1361_ECG = 12,
	IEC61966_2_1 = 13,
	BT2020_10 = 14,
	BT2020_12 = 15,
	SMPTE2084 = 16,
	SMPTEST2084 = SMPTE2084,
	SMPTE428 = 17,
	SMPTEST428_1 = SMPTE428,
	ARIB_STD_B67 = 18,
	S_LOG = 19,
	S_LOG2 = 20,
	S_LOG3 = 21,
	D_LOG = 22,
	APPLE_LOG_PROFILE = 23,
	CANON_LOG = 24,
	CANON_LOG2 = 25,
	CANON_LOG3 = 26,
	GAMMA24 = 27,
	GAMMA26 = 28,
	F_LOG = 29,
	F_LOG2 = 30,
	V_LOG = 31,
	N_LOG = 32,
	L_LOG = 33,
	RED_LOG = 34,
	RED_LOG_FILM = 35,
	LOG3G10 = 36,
	ARRILOGC3_SUP2X = 37,
	ARRILOGC3_SUP3X = 38,
	ARRILOGC4 = 39,
};

enum class ToneMapping
{
	Unspecified,
	ACES,
	NAES,
	Uncharted2,
	Reinhard,
	GranTurismo,
	Log2,
	Lottes,
	Uchimura,
};

class ProgressListener : public IObject
{
public:
	ProgressListener() :
	    completed{},
	    canceled{},
	    paused{},
	    progress{}
	{
	}

	void Complete()
	{
		completed.store(true);
	}

	bool IsCompleted() const
	{
		return completed.load();
	}

	void Cancel()
	{
		canceled.store(true);
	}

	bool IsCanceled() const
	{
		return canceled.load();
	}

	void Pause()
	{
		paused.store(true);
	}

	void Resume()
	{
		paused.store(false);
	}

	void Reset()
	{
		canceled.store(false);
		paused.store(false);
		completed.store(false);
		progress.store(0.0f);
	}

	bool IsPaused() const
	{
		return paused.load();
	}

	float GetProgress() const
	{
		return progress.load();
	}

	void SetProgress(float value)
	{
		progress.store(value);
	}

private:
	std::atomic<bool> canceled;

	std::atomic<bool> paused;

	std::atomic<bool> completed;

	std::atomic<float> progress;
};

struct ImageEncodeInfo
{
	int quality = 95;
	Ref<ProgressListener> listener;
};

enum class CodecId
{
	None,
	AVC,
	HEVC,
	VVC,
	VP9,
	AV1,
	H264_D3D12ENCODE,
	H264_NVENC,
	H264_QSV,
	HEVC_D3D12ENCODE,
	HEVC_NVENC,
	HEVC_QSV,
	VVC_QSV,
	VP9_QSV,
	AV1_NVENC,
	AV1_QSV,
	AAC,
	FLAC,
	TIFF,
	PNG,
	WEBP,
	JPEGXL,
	AVIF,
	MJPEG,
	JPEG2000,
	DPX,
	TARGA,
	PCX,
	EXR,
	SGI,
	SUNRASTER,
	JPEGLS,
	FITS,
	IFF_ILBM,
	XBM_IMAGE,
	XFACE,
	QDRAW,
	MPEG4,
	RAW,
	PCM_S16,
	H264 = AVC,
	H265 = HEVC,
	H266 = VVC,
};

struct CodecInfo
{
	void *handle = nullptr;
	MediaType mediaType = MediaType::Video;
	CodecId codecId = CodecId::None;
	union
	{
		uint32_t width = 0;
		uint32_t sampleRate;
	};

	union
	{
		uint32_t height = 0;
		Vision::ChannelLayout channelLayout;
	};

	Format format = Format::None;
	int bitRate = 0;
	int gopSize = 0;
	Rational framerate{};
	Rational timeBase{};
	Rational sampleAspectRatio{};
	Vision::DisplayOrientation displayOrientation{};
	std::string encoderPreset;
	std::string rateControl;
	int crf = 0;
	int maxBitRate = 0;
	int bufferSize = 0;
};

}
