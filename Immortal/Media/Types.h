#pragma once

#include "Format.h"

namespace Immortal
{
namespace Media
{

struct Description
{
    size_t  Size;
    size_t  Spatial;
    int32_t Width;
    int32_t Height;
    int32_t Depth;
    Format  Format;
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
