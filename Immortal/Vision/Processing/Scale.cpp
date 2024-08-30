/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */


#include "Scale.h"
#include "Common/SamplingFactor.h"
#include <algorithm>

namespace Immortal
{
namespace Vision
{

constexpr float kBicubicConvolutionKey = -0.5f;
float BicubicConvolutionKernal(float x)
{
	float weight;

	float absx = std::abs(x);
	if (absx <= 1)
	{
		weight = (float) ((kBicubicConvolutionKey + 2.0) * pow(absx, 3.0) - (kBicubicConvolutionKey + 3.0) * pow(absx, 2.0) + 1.0);
	}
	else if (absx < 2)
	{
		weight = (float) (kBicubicConvolutionKey * pow(absx, 3.0) - (kBicubicConvolutionKey * 5.0) * pow(absx, 2.0) + (kBicubicConvolutionKey * 8.0) * absx - (kBicubicConvolutionKey * 4.0));
	}
	else if (absx == 0)
	{
		weight = 1.0;
	}
	else
	{
		weight = 0;
	}

	return weight;
}

template <class T>
void TBicubicConvolutionInterpolate(T *dst, size_t dstStride, T *src, size_t srcStride, uint32_t dstWidth, uint32_t dstHeight, uint32_t srcWidth, uint32_t srcHeight)
{
	float x, y;

	int x0, x1, x2, x3;
	int y0, y1, y2, y3;
	float cx0, cx1, cx2, cx3;
	float cy0, cy1, cy2, cy3;

	float widthRatio  = (float)dstWidth  / srcWidth;
	float heightRatio = (float)dstHeight / srcHeight;

	if constexpr (std::is_same_v<T, uint16_t>)
	{
		srcStride >>= 1;
		dstStride >>= 1;
	}

	for (int iy = 0; iy < dstHeight; iy++)
	{
		T *data = &dst[iy * dstStride];
		for (int ix = 0; ix < dstWidth; ix++)
		{
			x = ix / widthRatio;
			y = iy / heightRatio;

			x1 = ((int)x);
			x0 = x1 - 1;
			x2 = x1 + 1;
			x3 = x2 + 1;
			y1 = ((int)y);
			y0 = y1 - 1;
			y2 = y1 + 1;
			y3 = y2 + 1;

			cx0 = BicubicConvolutionKernal(x - x0);
			cx1 = BicubicConvolutionKernal(x - x1);
			cx2 = BicubicConvolutionKernal(x - x2);
			cx3 = BicubicConvolutionKernal(x - x3);
			cy0 = BicubicConvolutionKernal(y - y0);
			cy1 = BicubicConvolutionKernal(y - y1);
			cy2 = BicubicConvolutionKernal(y - y2);
			cy3 = BicubicConvolutionKernal(y - y3);

			x0 = x0 < 0 ? x1 : x0;
			x3 = x3 >= srcWidth ? x2 : x3;
			y0 = y0 < 0 ? y1 : y0;
			y2 = y2 >= srcHeight ? y1 : y2;
			y3 = y3 >= srcHeight ? y2 : y3;

			float sample  = src[(size_t)y0 * srcStride + x0] * cx0 * cy0
                          + src[(size_t)y1 * srcStride + x0] * cx0 * cy1
                          + src[(size_t)y2 * srcStride + x0] * cx0 * cy2
                          + src[(size_t)y3 * srcStride + x0] * cx0 * cy3
                          + src[(size_t)y0 * srcStride + x1] * cx1 * cy0
                          + src[(size_t)y1 * srcStride + x1] * cx1 * cy1
                          + src[(size_t)y2 * srcStride + x1] * cx1 * cy2
                          + src[(size_t)y3 * srcStride + x1] * cx1 * cy3
                          + src[(size_t)y0 * srcStride + x2] * cx2 * cy0
                          + src[(size_t)y1 * srcStride + x2] * cx2 * cy1
                          + src[(size_t)y2 * srcStride + x2] * cx2 * cy2
                          + src[(size_t)y3 * srcStride + x2] * cx2 * cy3
                          + src[(size_t)y0 * srcStride + x3] * cx3 * cy0
                          + src[(size_t)y1 * srcStride + x3] * cx3 * cy1
                          + src[(size_t)y2 * srcStride + x3] * cx3 * cy2
                          + src[(size_t)y3 * srcStride + x3] * cx3 * cy3;

			data[ix] = (T)std::clamp(sample, 0.0f, (float)std::numeric_limits<T>::max());
		}
	}
}

void BicubicConvolutionInterpolate(Picture &dst, const Picture &src)
{
	SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
	GetSamplingFactor(dst.GetFormat(), factors);

	if (src.GetFormat().IsType(Format::HightBitDepth))
	{
		for (size_t i = 0; src.GetData(i); i++)
		{
			uint16_t *pDst = (uint16_t *)dst.GetData(i);
			uint16_t *pSrc = (uint16_t *)src.GetData(i);
			TBicubicConvolutionInterpolate<uint16_t>(
				pDst,
				dst.GetStride(i),
				pSrc,
				src.GetStride(i),
				dst.GetWidth()  >> factors[i].x,
				dst.GetHeight() >> factors[i].y,
			    src.GetWidth()  >> factors[i].x,
			    src.GetHeight() >> factors[i].y);
		}
	}
	else
	{
		for (size_t i = 0; src.GetData(i); i++)
		{
			TBicubicConvolutionInterpolate<uint8_t>(
				dst.GetData(i),
				dst.GetStride(i),
				src.GetData(i),
				src.GetStride(i),
				dst.GetWidth()  >> factors[i].x,
				dst.GetHeight() >> factors[i].y,
			    src.GetWidth()  >> factors[i].x,
			    src.GetHeight() >> factors[i].y);
		}
	}
}

void ScaleTo8Bits(Picture &dst, const Picture &src)
{
	auto &format = src.GetFormat();
	if (!format.IsType(Format::HightBitDepth))
	{
		return;
	}

	float scale = 1.0f;
	if (format.IsType(Format::_10Bits))
	{
		scale = 1.0f / 1023.0f;
	}
	else if (format.IsType(Format::_12Bits))
	{
		scale = 1.0f / 4095.0f;
	}
	else if (format.IsType(Format::_16Bits))
	{
		scale = 1.0f / 65535.0f;
	}
	scale *= 255.0f;

	for (size_t i = 0; src.GetData(i); i++)
	{
		SamplingFactor factors[SamplingFactor::kMaxSublayer];
		GetSamplingFactor(format, factors);

		auto width  = src.GetWidth()  >> factors[i].x;
		auto height = src.GetHeight() >> factors[i].y;
		for (size_t y = 0; y < height; y++)
		{
			auto dstStride = dst.GetStride(i);
			auto srcStride = src.GetStride(i);

			uint8_t  *pDst = &(dst.GetData(i)[y * dstStride]);
			uint16_t *pSrc = (uint16_t *)(&(src.GetData(i)[y * srcStride]));
			for (size_t x = 0; x < width; x++)
			{
				pDst[x] = (uint8_t)std::clamp(pSrc[x] * scale, 0.0f, 255.0f);
			}
		}
	}
}

}
}
