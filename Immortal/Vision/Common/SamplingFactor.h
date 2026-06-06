#pragma once

#include "Format.h"

namespace Immortal
{

struct SamplingFactor
{
	static constexpr size_t kMaxSublayer = 4;

	SamplingFactor(uint32_t x = 0, uint32_t y = 0) :
        x{ x },
        y{ y }
    {

    }

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
            x = 1;
            y = 0;
            break;

        case Format::YUV420P:
		case Format::YUV420P10:
        case Format::NV12:
        case Format::P010LE:
            x = 1;
            y = 1;
			break;

        default:
            break;
        }
    }

    uint32_t x = 0;
    uint32_t y = 0;
};

static inline void GetSamplingFactor(Format::ValueType format, SamplingFactor *factors)
{
	switch (format)
	{
		case Format::YUV444P:
		case Format::YUV444P10:
		case Format::YUV444P12:
		case Format::YUV444P16:
			factors[0] = {};
			factors[1] = {};
			factors[2] = {};
			break;

		case Format::YUV422P:
		case Format::YUV422P10:
		case Format::YUV422P12:
		case Format::YUV422P16:
			factors[0] = {};
			factors[1] = { 1, 0 };
			factors[2] = { 1, 0 };
			break;

		case Format::YUV420P:
		case Format::YUV420P10:
		case Format::YUV420P12:
		case Format::YUV420P16:
		case Format::NV12:
		case Format::P010LE:
			factors[0] = {};
			factors[1] = { 1, 1 };
			factors[2] = { 1, 1 };
			break;
		case Format::Y210:
		case Format::Y216:
			factors[0] = { 1, 0 };
			break;

		default:
			break;
	}
}

}
