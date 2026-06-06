#pragma once

#include "Format.h"

namespace Immortal
{
namespace Vision
{

struct Description
{
public:
    Description() :
        width{ 0 },
        height{ 0 },
        format{ Format::None }
    {

    }

    Description(uint32_t width, uint32_t height, Format format) :
        width{ width },
        height{ height },
        format{ format }
    {

    }

    size_t Spatial() const
    {
        return width * height;
    }

    size_t Size() const
    {
        return Spatial() * format.ComponentCount() * format.ElementSize();
    }

    union 
    {
        uint32_t width;
        uint32_t samples;
    };

    uint32_t height;
    Format  format;
};

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

template <class T>
struct PropertyeTypeGetter
{
	static constexpr PropertyType type = PropertyType::Unspecified;
};

#define DEFINE_PROPERTY_GETTER(T, U) template <> struct PropertyeTypeGetter<T> { static constexpr PropertyType type = PropertyType::U; };
DEFINE_PROPERTY_GETTER(DisplayOrientation, DisplayOrientation)

struct TrackInfo
{
	std::string name;
	int streamIndex;
};

}

enum class MediaType
{
    Video    = 0,
    Audio    = 1,
    Data     = 2,
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
    Reserved0         = 0,
    BT709             = 1,
	Unspecified       = 2,
    Reserved          = 3,
    GAMMA22           = 4,
    GAMMA28           = 5,
	SMPTE170M         = 6,
	SMPTE240M         = 7,
    LINEAR            = 8,
    LOG               = 9,
    LOG_SQRT          = 10,
	IEC61966_2_4      = 11,
	BT1361_ECG        = 12,
	IEC61966_2_1      = 13,
    BT2020_10         = 14,
    BT2020_12         = 15,
	SMPTE2084         = 16,
	SMPTEST2084       = SMPTE2084,
	SMPTE428          = 17,
	SMPTEST428_1      = SMPTE428,
	ARIB_STD_B67      = 18,
    S_LOG             = 19,
    S_LOG2            = 20,
    S_LOG3            = 21,
    D_LOG             = 22,
    APPLE_LOG_PROFILE = 23,
    CANON_LOG         = 24,
    CANON_LOG2        = 25,
    CANON_LOG3        = 26,
    GAMMA24           = 27,
    GAMMA26           = 28,
    F_LOG             = 29,
    F_LOG2            = 30,
    V_LOG             = 31,
    N_LOG             = 32,
    L_LOG             = 33,
    RED_LOG           = 34,
    RED_LOG_FILM      = 35,
    LOG3G10           = 36,
    ARRILOGC3_SUP2X   = 37,
	ARRILOGC3_SUP3X   = 38,
    ARRILOGC4         = 39,
};

enum class ToneMapping
{
    ACES,
};

}
