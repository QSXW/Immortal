#include "ColorSpace.h"
#include "slintrinsic.h"

namespace Immortal
{
namespace Vision
{

namespace ColorSpace
{

void ColorSpace::RGBA8ToYUVA4444(uint8_t *dst, const uint8_t *src, size_t size)
{
    auto *srcptr = src;
    auto *dstptr = dst;
    for (size_t i = 0; i < size; i += 4, dstptr += 4, srcptr += 4)
    {
        rgb2yuv<uint8_t, CoefficientType::REC601>(
            dstptr[0],
            dstptr[1],
            dstptr[2],
            srcptr[0],
            srcptr[1],
            srcptr[2]
            );
        dstptr[3] = srcptr[3];
    }
}

void ColorSpace::RGBA8ToYUVA4444_AVX2(uint8_t *dst, const uint8_t *src, size_t size)
{
    auto coeffients = ColorSpace::Get<CoefficientType::REC601>();
}

}

}
}
