#pragma once

#include "Format.h"

namespace Immortal
{

struct SamplingFactor
{
    SamplingFactor(Format::ValueType format)
    {
        switch (format)
        {
        case Format::YUV444P:
        case Format::YUV444P10:
            x = 0;
            y = 0;
            break;

        case Format::YUV422P:
        case Format::YUV422P10:
            x = 0;
            y = 1;
            break;

        case Format::YUV420P:
		case Format::YUV420P10:
        case Format::NV12:
        case Format::P010LE:
            x = 1;
            y = 1;

        default:
            break;
        }
    }

    uint32_t x = 0;
    uint32_t y = 0;
};

}
