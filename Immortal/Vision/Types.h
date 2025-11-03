#pragma once

#include "Format.h"
#include "Math/Math.h"

#include <string>
#include <vector>
#include <cstdint>

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
		completed = true;
	}

	void Cancel()
	{
		canceled = true;
	}

	void Pause()
	{
		paused = true;
	}

	float GetProgress() const
	{
		return progress;
	}

	void SetProgress(float value)
	{
		progress = value;
	}

private:
	std::atomic<bool> canceled;

	std::atomic<bool> paused;

	std::atomic<bool> completed;

	float progress;
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
	MPEG4,
	RAW,
	H264 = AVC,
	H265 = HEVC,
	H266 = VVC,
};

struct EncodeInfo
{
	MediaType mediaType;
	CodecId codecId;
	union
	{
		uint32_t width;
		uint32_t sampleRate;
	};

	union
	{
		uint32_t height;
		uint32_t channels;
	};

	Format format;
	int bitRate;
	int gopSize;
	Rational framerate;
	Rational timeBase;
	Vision::DisplayOrientation displayOrientation;
};

}
