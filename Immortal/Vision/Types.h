#pragma once

#include "Format.h"

namespace Immortal
{
namespace Vision
{

enum class ColorSpace
{
    YUV,
    YUV_BT709,
};

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

}

enum class MediaType
{
    Video    = 0,
    Audio    = 1,
    Data     = 2,
    Subtitle = 3
};

}
