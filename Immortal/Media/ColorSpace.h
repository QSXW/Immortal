#pragma once

#include <cstdint>

namespace Immortal
{
namespace Media
{

namespace ColorSpace
{

enum class CoefficientType : size_t
{
    REC601 = 0,
    REC709
};

static float Coefficients[][4] = {
    {  0.299,  0.587, 0.114, 0 }, /* ITU-R Rec. 601 */
    { 0.2125, 0.7157, 0.721, 0 }, /* ITU-R Rec. 709 (1990) */
};

template <CoefficientType type>
float* Get()
{
    return Coefficients[static_cast<size_t>(type)];
}

void yuv2rgb(const uint8_t *src, size_t size, uint8_t *dst);

void RGBA8ToYUVA4444(uint8_t *dst, const uint8_t *src, size_t size);

void RGBA8ToYUVA4444_AVX2(uint8_t *dst, const uint8_t *src, size_t size);

};

}
}

