#pragma once

#include <cstdint>

namespace Media
{
namespace ColorSpace
{

static int32_t YUV2RGBCoefficients[][4] = {
    { 117489, 138438, 13975, 34925 }, /* ITU-R Rec. 709 (1990) */
};

void yuv2rgb(const uint8_t *src, size_t size, uint8_t *dst);

};
}
