#pragma once

#include "Format.h"

namespace Immortal
{
namespace Vision
{

enum class ColorSpace
{
    YUV420P,
    YUV422P,
    YUV444P,
    RGBA32,
    NV12,
    None
};

struct Description
{
public:
    Description() :
        width{ 0 },
        height{ 0 },
        format{ Format::None },
        colorSpace{ ColorSpace::None }
    {

    }

    size_t Spatial() const
    {
        return width * height;
    }

    size_t Size() const
    {
        return Spatial() * format.ComponentCount();
    }

    int32_t width;
    int32_t height;
    Format  format;
    ColorSpace colorSpace;
};

enum class Type
{
    Unspecifed,
    BMP,
    JPEG,
    PNG,
    EXIF,
    MP4,
    HEVC
};

}
}
