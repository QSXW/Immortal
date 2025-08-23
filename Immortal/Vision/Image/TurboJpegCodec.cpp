#include "TurboJpegCodec.h"

#if HAVE_TURBOJPEG
#include <turbojpeg.h>
#include <jpeglib.h>
#endif

namespace Immortal
{
namespace Vision
{

#if HAVE_TURBOJPEG

static inline TJSAMP GetSampling(const Format &format)
{
	switch (Format::ValueType(format))
	{
		case Format::YUV420P:
		case Format::YUV420P10:
		case Format::YUV420P12:
		case Format::YUV420P16:
			return TJSAMP_420;

		case Format::YUV422P:
		case Format::YUV422P10:
		case Format::YUV422P12:
		case Format::YUV422P16:
			return TJSAMP_422;

		case Format::YUV444P:
		case Format::YUV444P10:
		case Format::YUV444P12:
		case Format::YUV444P16:
			return TJSAMP_444;

		case Format::R8_UNORM:
		case Format::RGBA8:
			return TJSAMP_420;

		default:
			return TJSAMP_UNKNOWN;
	}
}

TurboJpegCodec::TurboJpegCodec(bool isOutputYUV) :
    Super{},
    numerator{ 1 },
	denominator{ 1 },
    isOutputYUV{ isOutputYUV },
    quality{ 90 },
    desireSize{}
{

}

TurboJpegCodec::~TurboJpegCodec()
{

}

CodecError TurboJpegCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.GetBuffer();

    return Decode(buffer.data(), buffer.size());
}

CodecError TurboJpegCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	auto sampling = GetSampling(picture.GetFormat());
	if (sampling == TJSAMP_UNKNOWN)
	{
		LOG::ERR("Unsupported format");
		return CodecError::InvalidArguments;
	}

	tjhandle handle = tjInitCompress();
	if (!handle)
	{
		LOG::ERR("Failed to initialize turbo jpeg compress handle!");
		return CodecError::ExternalFailed;
	}

	struct CompressedRef
	{
		CompressedRef() :
			data{},
			size{}
		{

		}

		~CompressedRef()
		{
			if (data)
			{
				tjFree(data);
				data = nullptr;
			}
		}

		uint8_t *data;
		unsigned long size;
	};

	CompressedRef compressedRef{};

	uint32_t width  = picture.GetWidth();
	uint32_t height = picture.GetHeight();

	Format format = picture.GetFormat();
	if (format.IsType(Format::YUV))
	{	
		int strides[4] = {};
		uint8_t *planes[4] = {};
		for (size_t i = 0; picture.GetData(i); i++)
		{
			strides[i] = picture.GetStride(i);
			planes[i]  = picture.GetData(i);
		}

		tjCompressFromYUVPlanes(handle, (const unsigned char **)planes, width, strides, height, sampling, &compressedRef.data, &compressedRef.size, quality, 0);
	}
	else
	{
		int pixelFormat = TJPF_RGBX;
		if (format == Format::R8_UNORM || format == Format::R16_UNORM)
		{
			pixelFormat = TJPF_GRAY;
		}
		tjCompress2(handle, picture.GetData(0), width, picture.GetStride(0), height, pixelFormat, &compressedRef.data, &compressedRef.size, sampling, quality, 0);
	}

	codedFrame = CodedFrame{compressedRef.data, compressedRef.size};

	tjDestroy(handle);
    return CodecError::Success;
}
#endif

CodecError TurboJpegCodec::Decode(const uint8_t *data, size_t size)
{
#if HAVE_TURBOJPEG
    tjhandle handle = {};

    handle = tjInitDecompress();
    if (!handle)
    {
        LOG::ERR("Failed to initialize turbo jpeg decompress handle!");
        return CodecError::ExternalFailed;
    }

    tjscalingfactor scalingFactor = {1, 1};
	int width, height, subsample, colorSpace;
	int result = tjDecompressHeader3(handle, data, size, &width, &height, &subsample, &colorSpace);
	if (result != 0)
	{
		LOG::ERR("Failed to decompress JPEG header: {}", tjGetErrorStr2(handle));
		tjDestroy(handle);
		return CodecError::ExternalFailed;
	}

    if (desireSize > 0)
    {       
		int size = std::max(width, height);
		if (size > desireSize)
        {
			float ratio = size / (float) desireSize;
            if (ratio >= 2.0f)
            {
                scalingFactor.num = 1;
                scalingFactor.denom = std::clamp(int((ratio + 1.0f) / 2.0f) * 2, 1, 8);
            }
        }  
    }
    else if (numerator != 1 || denominator != 1)
    {
        scalingFactor.num   = numerator;
        scalingFactor.denom = denominator;
    }
    
    result = tj3SetScalingFactor(handle, scalingFactor);
    if (result != 0)
    {
        LOG::ERR("Failed to set scaling factor: {}", tjGetErrorStr2(handle));
        tjDestroy(handle);
        return CodecError::ExternalFailed;
    }

    width  = TJSCALED(width,  scalingFactor);
	height = TJSCALED(height, scalingFactor);

    Format format = Format::RGBA8;
    if (isOutputYUV)
    {
        format = Format::YUV420P;
        if (subsample == TJSAMP_422)
        {
            format = Format::YUV422P;
        }
        else if (subsample == TJSAMP_444)
        {
            format = Format::YUV444P;
        }
    }
	
    picture = Picture{(uint32_t)width, (uint32_t)height, format, true, {PropertyType::DisplayOrientation}};

    if (isOutputYUV)
    {
        result = tjDecompressToYUV2(handle, data, size, picture.GetData(), width, 8, height, 0);
    }
    else
    {
        auto pitch = picture.GetStride(0);
        result = tjDecompress2(handle, data, size, picture.GetData(), width, pitch, height, TJPF_RGBX, 0);
    }

    if (result != 0)
    {
        LOG::ERR("Failed to decompress JPEG data: {}", tjGetErrorStr2(handle));
        tjDestroy(handle);
        return CodecError::ExternalFailed;
    }

    tjDestroy(handle);
    return CodecError::Success;
#else
    return CodecError::NotImplement;
#endif
}

void TurboJpegCodec::SetScale(int _numerator, int _denominator)
{
#if HAVE_TURBOJPEG
	numerator   = _numerator;
	denominator = _denominator;
#endif
}

void TurboJpegCodec::SetQuality(int value)
{
	quality = value;
}

void TurboJpegCodec::SetDesireSize(int value)
{
	desireSize = value;
}

}
}
