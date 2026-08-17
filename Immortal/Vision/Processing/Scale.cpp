/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */


#include "Scale.h"
#include "Common/SamplingFactor.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

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

template <class T, size_t offset = 0, size_t elements = 1>
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

			float sample  = src[(size_t)y0 * srcStride + (x0 * elements + offset)] * cx0 * cy0
                          + src[(size_t)y1 * srcStride + (x0 * elements + offset)] * cx0 * cy1
                          + src[(size_t)y2 * srcStride + (x0 * elements + offset)] * cx0 * cy2
                          + src[(size_t)y3 * srcStride + (x0 * elements + offset)] * cx0 * cy3
                          + src[(size_t)y0 * srcStride + (x1 * elements + offset)] * cx1 * cy0
                          + src[(size_t)y1 * srcStride + (x1 * elements + offset)] * cx1 * cy1
                          + src[(size_t)y2 * srcStride + (x1 * elements + offset)] * cx1 * cy2
                          + src[(size_t)y3 * srcStride + (x1 * elements + offset)] * cx1 * cy3
                          + src[(size_t)y0 * srcStride + (x2 * elements + offset)] * cx2 * cy0
                          + src[(size_t)y1 * srcStride + (x2 * elements + offset)] * cx2 * cy1
                          + src[(size_t)y2 * srcStride + (x2 * elements + offset)] * cx2 * cy2
                          + src[(size_t)y3 * srcStride + (x2 * elements + offset)] * cx2 * cy3
                          + src[(size_t)y0 * srcStride + (x3 * elements + offset)] * cx3 * cy0
                          + src[(size_t)y1 * srcStride + (x3 * elements + offset)] * cx3 * cy1
                          + src[(size_t)y2 * srcStride + (x3 * elements + offset)] * cx3 * cy2
                          + src[(size_t)y3 * srcStride + (x3 * elements + offset)] * cx3 * cy3;

			data[ix * elements + offset] = (T) std::clamp(sample, 0.0f, (float) std::numeric_limits<T>::max());
		}
	}
}

template <class T>
void RGBAMaskAlphaChannel(Picture &picture)
{
	auto &width  = picture.GetWidth();
	auto &height = picture.GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			T &alpha = (T &)picture.GetData(0)[y * picture.GetStride(0) + (x * 4 + 3) * sizeof(T)];
			alpha = std::numeric_limits<T>::max();
		}
	}
}

void BicubicConvolutionInterpolate(Picture &dst, const Picture &src)
{
	SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
	GetSamplingFactor(dst.GetFormat(), factors);

	auto &format = src.GetFormat();

	if (format.IsType(Format::YUV))
	{
		if (format.IsType(Format::HightBitDepth))
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
	else if (format == Format::RGBA8 || format == Format::BGRA8)
	{
		TBicubicConvolutionInterpolate<uint8_t, 0, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint8_t, 1, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint8_t, 2, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint8_t, 3, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
	else if (format == Format::RGBA16 || format == Format::R16G16B16A16_UINT)
	{
		TBicubicConvolutionInterpolate<uint16_t, 0, 4>((uint16_t *)dst.GetData(0), dst.GetStride(0), (uint16_t *)src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint16_t, 1, 4>((uint16_t *)dst.GetData(0), dst.GetStride(0), (uint16_t *)src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint16_t, 2, 4>((uint16_t *)dst.GetData(0), dst.GetStride(0), (uint16_t *)src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint16_t, 3, 4>((uint16_t *)dst.GetData(0), dst.GetStride(0), (uint16_t *)src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
	else if (format == Format::R8G8B8_UNORM || format == Format::B8G8R8_UNORM)
	{
		TBicubicConvolutionInterpolate<uint8_t, 0, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint8_t, 1, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TBicubicConvolutionInterpolate<uint8_t, 2, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
}

template <class T, size_t offset = 0, size_t elements = 1>
bool TNearestInterpolateLayoutIsValid(const T *dst, size_t dstStride, const T *src, size_t srcStride, uint32_t dstWidth, uint32_t dstHeight, uint32_t srcWidth, uint32_t srcHeight)
{
	static_assert(elements > 0);
	static_assert(offset < elements);

	if (!dst || !src || dstWidth == 0 || dstHeight == 0 || srcWidth == 0 || srcHeight == 0)
	{
		return false;
	}
	if ((reinterpret_cast<std::uintptr_t>(dst) % alignof(T)) != 0 ||
		(reinterpret_cast<std::uintptr_t>(src) % alignof(T)) != 0)
	{
		return false;
	}

	if ((dstStride % sizeof(T)) != 0 || (srcStride % sizeof(T)) != 0)
	{
		return false;
	}
	dstStride /= sizeof(T);
	srcStride /= sizeof(T);

	if (dstStride == 0 || srcStride == 0)
	{
		return false;
	}

	constexpr size_t maximum = std::numeric_limits<size_t>::max();
	auto minimumStride = [] (uint32_t width, size_t &value) {
		const size_t lastPixel = size_t(width) - 1;
		if (lastPixel > (std::numeric_limits<size_t>::max() - offset - 1) / elements)
		{
			return false;
		}
		value = lastPixel * elements + offset + 1;
		return true;
	};

	size_t dstMinimumStride = 0;
	size_t srcMinimumStride = 0;
	if (!minimumStride(dstWidth, dstMinimumStride) || !minimumStride(srcWidth, srcMinimumStride))
	{
		return false;
	}
	if (dstStride < dstMinimumStride || srcStride < srcMinimumStride)
	{
		return false;
	}

	const size_t dstRows = size_t(dstHeight) - 1;
	const size_t srcRows = size_t(srcHeight) - 1;
	if ((dstRows != 0 && dstStride > (maximum - dstMinimumStride) / dstRows) ||
		(srcRows != 0 && srcStride > (maximum - srcMinimumStride) / srcRows))
	{
		return false;
	}

	const size_t dstElements = dstRows * dstStride + dstMinimumStride;
	const size_t srcElements = srcRows * srcStride + srcMinimumStride;
	const size_t maximumPointerElements = static_cast<size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(T);
	if (dstElements > maximumPointerElements || srcElements > maximumPointerElements)
	{
		return false;
	}

	auto addressRangeIsValid = [] (const T *data, size_t elementCount) {
		const size_t lastByteOffset = (elementCount - 1) * sizeof(T) + sizeof(T) - 1;
		const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(data);
		return lastByteOffset <= std::numeric_limits<std::uintptr_t>::max() - address;
	};
	if (!addressRangeIsValid(dst, dstElements) || !addressRangeIsValid(src, srcElements))
	{
		return false;
	}

	return true;
}

template <class T, size_t offset = 0, size_t elements = 1>
bool TNearestInterpolate(T *dst, size_t dstStride, const T *src, size_t srcStride, uint32_t dstWidth, uint32_t dstHeight, uint32_t srcWidth, uint32_t srcHeight)
{
	if (!TNearestInterpolateLayoutIsValid<T, offset, elements>(dst, dstStride, src, srcStride, dstWidth, dstHeight, srcWidth, srcHeight))
	{
		return false;
	}

	dstStride /= sizeof(T);
	srcStride /= sizeof(T);

	for (uint32_t iy = 0; iy < dstHeight; iy++)
	{
		const uint32_t srcY = std::min<uint32_t>(
			srcHeight - 1,
			static_cast<uint32_t>((uint64_t(iy) * srcHeight) / dstHeight));
		T *dstRow = dst + size_t(iy) * dstStride;
		const T *srcRow = src + size_t(srcY) * srcStride;
		for (uint32_t ix = 0; ix < dstWidth; ix++)
		{
			const uint32_t srcX = std::min<uint32_t>(
				srcWidth - 1,
				static_cast<uint32_t>((uint64_t(ix) * srcWidth) / dstWidth));
			dstRow[size_t(ix) * elements + offset] = srcRow[size_t(srcX) * elements + offset];
		}
	}

	return true;
}

void NearestInterpolate(Picture &dst, const Picture &src)
{
	if (!dst || !src ||
		dst.GetWidth() == 0 || dst.GetHeight() == 0 ||
		src.GetWidth() == 0 || src.GetHeight() == 0 ||
		dst.GetFormat() != src.GetFormat() ||
		dst.GetMemoryType() != PictureMemoryType::System ||
		src.GetMemoryType() != PictureMemoryType::System)
	{
		return;
	}

	SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
	GetSamplingFactor(dst.GetFormat(), factors);

	auto &format = src.GetFormat();

	if (format == Format::Y210 || format == Format::Y216)
	{
		const uint32_t dstPackedWidth = dst.GetWidth() / 2 + dst.GetWidth() % 2;
		const uint32_t srcPackedWidth = src.GetWidth() / 2 + src.GetWidth() % 2;
		TNearestInterpolate<uint64_t>(
			(uint64_t *)dst.GetData(0), dst.GetStride(0),
			(const uint64_t *)src.GetData(0), src.GetStride(0),
			dstPackedWidth, dst.GetHeight(), srcPackedWidth, src.GetHeight());
	}
	else if (format.IsType(Format::YUV))
	{
		const size_t planeCount = format.IsType(Format::NV) ? 2 : (format == Format::YUVA420P ? 4 : 3);
		for (size_t i = 0; i < planeCount; ++i)
		{
			if (!dst.GetData(i) || !src.GetData(i))
			{
				return;
			}

			const uint32_t dstPlaneWidth = dst.GetWidth() >> factors[i].x;
			const uint32_t dstPlaneHeight = dst.GetHeight() >> factors[i].y;
			const uint32_t srcPlaneWidth = src.GetWidth() >> factors[i].x;
			const uint32_t srcPlaneHeight = src.GetHeight() >> factors[i].y;
			const bool valid = format.IsType(Format::HightBitDepth)
				? ((format.IsType(Format::NV) && i == 1)
					? TNearestInterpolateLayoutIsValid<uint16_t, 1, 2>(
						(const uint16_t *)dst.GetData(i), dst.GetStride(i),
						(const uint16_t *)src.GetData(i), src.GetStride(i),
						dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight)
					: TNearestInterpolateLayoutIsValid<uint16_t>(
						(const uint16_t *)dst.GetData(i), dst.GetStride(i),
						(const uint16_t *)src.GetData(i), src.GetStride(i),
						dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight))
				: ((format.IsType(Format::NV) && i == 1)
					? TNearestInterpolateLayoutIsValid<uint8_t, 1, 2>(
						dst.GetData(i), dst.GetStride(i), src.GetData(i), src.GetStride(i),
						dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight)
					: TNearestInterpolateLayoutIsValid<uint8_t>(
						dst.GetData(i), dst.GetStride(i), src.GetData(i), src.GetStride(i),
						dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight));
			if (!valid)
			{
				return;
			}
		}

		if (format.IsType(Format::HightBitDepth))
		{
			for (size_t i = 0; i < planeCount; i++)
			{
				uint16_t *pDst = (uint16_t *) dst.GetData(i);
				const uint16_t *pSrc = (const uint16_t *) src.GetData(i);
				const uint32_t dstPlaneWidth = dst.GetWidth() >> factors[i].x;
				const uint32_t dstPlaneHeight = dst.GetHeight() >> factors[i].y;
				const uint32_t srcPlaneWidth = src.GetWidth() >> factors[i].x;
				const uint32_t srcPlaneHeight = src.GetHeight() >> factors[i].y;
				if (format.IsType(Format::NV) && i == 1)
				{
					TNearestInterpolate<uint16_t, 0, 2>(pDst, dst.GetStride(i), pSrc, src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
					TNearestInterpolate<uint16_t, 1, 2>(pDst, dst.GetStride(i), pSrc, src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
				}
				else
				{
					TNearestInterpolate<uint16_t>(pDst, dst.GetStride(i), pSrc, src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
				}
			}
		}
		else
		{
			for (size_t i = 0; i < planeCount; i++)
			{
				const uint32_t dstPlaneWidth = dst.GetWidth() >> factors[i].x;
				const uint32_t dstPlaneHeight = dst.GetHeight() >> factors[i].y;
				const uint32_t srcPlaneWidth = src.GetWidth() >> factors[i].x;
				const uint32_t srcPlaneHeight = src.GetHeight() >> factors[i].y;
				if (format.IsType(Format::NV) && i == 1)
				{
					TNearestInterpolate<uint8_t, 0, 2>(dst.GetData(i), dst.GetStride(i), src.GetData(i), src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
					TNearestInterpolate<uint8_t, 1, 2>(dst.GetData(i), dst.GetStride(i), src.GetData(i), src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
				}
				else
				{
					TNearestInterpolate<uint8_t>(dst.GetData(i), dst.GetStride(i), src.GetData(i), src.GetStride(i), dstPlaneWidth, dstPlaneHeight, srcPlaneWidth, srcPlaneHeight);
				}
			}
		}
	}
	else if (format == Format::RGBA8 || format == Format::BGRA8)
	{
		if (!TNearestInterpolateLayoutIsValid<uint8_t, 3, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight()))
		{
			return;
		}
		TNearestInterpolate<uint8_t, 0, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint8_t, 1, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint8_t, 2, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint8_t, 3, 4>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
	else if (format == Format::RGBA16 || format == Format::R16G16B16A16_UINT)
	{
		if (!TNearestInterpolateLayoutIsValid<uint16_t, 3, 4>((uint16_t *)dst.GetData(0), dst.GetStride(0), (const uint16_t *)src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight()))
		{
			return;
		}
		TNearestInterpolate<uint16_t, 0, 4>((uint16_t *) dst.GetData(0), dst.GetStride(0), (uint16_t *) src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint16_t, 1, 4>((uint16_t *) dst.GetData(0), dst.GetStride(0), (uint16_t *) src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint16_t, 2, 4>((uint16_t *) dst.GetData(0), dst.GetStride(0), (uint16_t *) src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint16_t, 3, 4>((uint16_t *) dst.GetData(0), dst.GetStride(0), (uint16_t *) src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
	else if (format == Format::R8G8B8_UNORM || format == Format::B8G8R8_UNORM)
	{
		if (!TNearestInterpolateLayoutIsValid<uint8_t, 2, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight()))
		{
			return;
		}
		TNearestInterpolate<uint8_t, 0, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint8_t, 1, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
		TNearestInterpolate<uint8_t, 2, 3>(dst.GetData(0), dst.GetStride(0), src.GetData(0), src.GetStride(0), dst.GetWidth(), dst.GetHeight(), src.GetWidth(), src.GetHeight());
	}
}

void ScaleTo8Bits(Picture &dst, const Picture &src)
{
	auto &format = src.GetFormat();
	if (!format.IsType(Format::HightBitDepth) && format != Format::R16G16B16A16_UNORM)
	{
		return;
	}

	int shift = 8;
	if (format.IsType(Format::_10Bits))
	{
		shift = 2;
	}
	else if (format.IsType(Format::_12Bits))
	{
		shift = 4;
	}
	else if (format.IsType(Format::_16Bits))
	{
		shift = 8;
	}

	SamplingFactor factors[SamplingFactor::kMaxSublayer];
	GetSamplingFactor(format, factors);

	for (size_t i = 0; src.GetData(i); i++)
	{
		auto width  = src.GetWidth()  >> factors[i].x;
		auto height = src.GetHeight() >> factors[i].y;
	
		width *= format.GetComponent();
		for (size_t y = 0; y < height; y++)
		{
			auto dstStride = dst.GetStride(i);
			auto srcStride = src.GetStride(i);

			uint8_t  *pDst = &(dst.GetData(i)[y * dstStride]);
			uint16_t *pSrc = (uint16_t *)(&(src.GetData(i)[y * srcStride]));
			for (size_t x = 0; x < width; x++)
			{
				pDst[x] = pSrc[x] >> shift;
			}
		}
	}
}

Picture RGBA8ToYUV420P(const Picture &picture)
{
	if (picture.GetFormat() != Format::RGBA8)
	{
		return {};
	}
	
	Picture dst{picture.GetWidth(), picture.GetHeight(), Format::YUV420P, true};
	uint8_t *src = picture.GetData();

	uint32_t width  = dst.GetWidth()  / 2;
	uint32_t height = dst.GetHeight() / 2;

	auto texelSize = picture.GetFormat().GetTexelSize();
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			uint8_t *rgba00 = &src[(y * 2    ) * picture.GetStride(0) + (x * 2    ) * texelSize];
			uint8_t *rgba01 = &src[(y * 2    ) * picture.GetStride(0) + (x * 2 + 1) * texelSize];
			uint8_t *rgba10 = &src[(y * 2 + 1) * picture.GetStride(0) + (x * 2    ) * texelSize];
			uint8_t *rgba11 = &src[(y * 2 + 1) * picture.GetStride(0) + (x * 2 + 1) * texelSize];
			uint8_t &Y00    = dst.GetData(0)[(y * 2    ) * dst.GetStride(0) + x * 2];
			uint8_t &Y01    = dst.GetData(0)[(y * 2    ) * dst.GetStride(0) + x * 2 + 1];
			uint8_t &Y10    = dst.GetData(0)[(y * 2 + 1) * dst.GetStride(0) + x * 2];
			uint8_t &Y11    = dst.GetData(0)[(y * 2 + 1) * dst.GetStride(0) + x * 2 + 1];
			uint8_t &U      = dst.GetData(1)[y * dst.GetStride(1) + x];
			uint8_t &V      = dst.GetData(2)[y * dst.GetStride(2) + x];

			int y00 = (0.299f * rgba00[0]) + (0.587f * rgba00[1]) + (0.114f * rgba00[2]);
			int y01 = (0.299f * rgba01[0]) + (0.587f * rgba01[1]) + (0.114f * rgba01[2]);
			int y10 = (0.299f * rgba10[0]) + (0.587f * rgba10[1]) + (0.114f * rgba10[2]);
			int y11 = (0.299f * rgba11[0]) + (0.587f * rgba11[1]) + (0.114f * rgba11[2]);

			Y00 = std::clamp(y00, 0, 255);
			Y01 = std::clamp(y01, 0, 255);
			Y10 = std::clamp(y10, 0, 255);
			Y11 = std::clamp(y11, 0, 255);

			auto r = (rgba00[0] + rgba01[0] + rgba10[0] + rgba11[0]) >> 2;
			auto g = (rgba00[1] + rgba01[1] + rgba10[1] + rgba11[1]) >> 2;
			auto b = (rgba00[2] + rgba01[2] + rgba10[2] + rgba11[2]) >> 2;
			auto y_value = (y00 + y01 + y10 + y11) >> 2;
			U = std::clamp(int(0.492f * (b - y_value) + 128), 0, 255);
			V = std::clamp(int(0.877f * (r - y_value) + 128), 0, 255);
		}
	}

	return dst;
}

}
}
