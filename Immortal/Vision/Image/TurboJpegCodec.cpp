#include "TurboJpegCodec.h"

#if HAVE_TURBOJPEG
#include <turbojpeg.h>
#include <jpeglib.h>
#endif
#include "ICC.h"

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

		default:
			return TJSAMP_UNKNOWN;
	}
}

static inline TJPF GetPixelFormat(Format format)
{
	if (format == Format::R8G8B8A8_UNORM)
	{
		return TJPF_RGBX;
	}
	else if (format == Format::B8G8R8A8_UNORM)
	{
		return TJPF_BGRX;
	}
	else if (format == Format::R8G8B8_UNORM)
	{
		return TJPF_RGB;
	}
	else if (format == Format::B8G8R8_UNORM)
	{
		return TJPF_BGR;
	}
	if (format == Format::R8_UNORM || format == Format::R16_UNORM)
	{
		return TJPF_GRAY;
	}

	return TJPF_UNKNOWN;
}

struct ProgressManager
{
	struct jpeg_progress_mgr pub;
	TurboJpegCodec *_this;
	float last_percent = 0;
};

METHODDEF(void)
progress_monitor(j_common_ptr cinfo)
{
	auto progress = (ProgressManager *) cinfo->progress;

	int percent;
	if (progress->pub.total_passes > 1)
	{
		percent = (progress->pub.completed_passes + (float) progress->pub.pass_counter / progress->pub.pass_limit) /
		                 progress->pub.total_passes;
	}
	else
	{
		percent = progress->pub.pass_counter / progress->pub.pass_limit;
	}

	if (percent != progress->last_percent)
	{
		progress->_this->info.listener->SetProgress(percent);
		progress->last_percent = percent;
	}
}

TurboJpegCodec::TurboJpegCodec(bool isOutputYUV) :
    Super{},
	ICLASS,
    numerator{ 1 },
	denominator{ 1 },
    isOutputYUV{ isOutputYUV },
    desireSize{},
    info{}
{

}

TurboJpegCodec::TurboJpegCodec(const ImageEncodeInfo &_info) :
    TurboJpegCodec{}
{
	info = _info;
}

TurboJpegCodec::~TurboJpegCodec()
{

}

CodecError TurboJpegCodec::Decode(const CodedFrame &codedFrame)
{
    return Decode(codedFrame.GetData(), codedFrame.GetSize());
}

CodecError TurboJpegCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	auto &format = picture.GetFormat();

	TJPF pixelFormat;
	TJSAMP sampling;
	if (format.IsType(Format::YUV))
	{
		sampling = GetSampling(format);
		if (sampling == TJSAMP_UNKNOWN)
		{
			CLOG_ERROR("Unknown sampling!");
			return CodecError::InvalidArguments;
		}
	}
	else
	{
		pixelFormat = GetPixelFormat(format);
		if (pixelFormat == TJPF_UNKNOWN)
		{
			CLOG_ERROR("Unknown format!");
			return CodecError::InvalidArguments;
		}
	}

	tjhandle handle = tjInitCompress();
	if (!handle)
	{
		CLOG_ERROR("Failed to initialize turbo jpeg compress handle!");
		return CodecError::ExternalFailed;
	}

	ProgressManager progress = {
	    .pub = {
	        .progress_monitor = progress_monitor,
		},
	    ._this = this,
		.last_percent = 0,
	};

	jpeg_compress_struct *cinfo = (jpeg_compress_struct *)handle;
	if (info.listener)
	{
		jpeg_simple_progression(cinfo);
		cinfo->progress = &progress.pub;
	}

	auto icc = picture.GetProperty<ICCProfileProperty>();
	if (icc)
	{
		auto &profile = icc->profile;
		tj3SetICCProfile(handle, profile.data(), profile.size());
	}

	uint32_t width  = picture.GetWidth();
	uint32_t height = picture.GetHeight();

	uint8_t *data = nullptr;
	unsigned long size = 0;
	if (format.IsType(Format::YUV))
	{
		tjCompressFromYUVPlanes(handle, (const unsigned char **) &picture.GetData(), width, (const int *) &picture.GetStride(), height, sampling, &data, &size, info.quality, 0);
	}
	else
	{
		tjCompress2(handle, picture.GetData(0), width, picture.GetStride(0), height, pixelFormat, &data, &size, TJSAMP_420, info.quality, 0);
	}

	if (info.listener)
	{
		info.listener->Complete();
		info.listener->SetProgress(1.0f);
	}

	codedFrame = CodedFrame{data, size};
	codedFrame.SetAnonymous(data);

	codedFrame.SetRelease([] (void *ptr) {
		tjFree((unsigned char *)ptr);
	});

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
        CLOG_ERROR("Failed to initialize turbo jpeg decompress handle!");
        return CodecError::ExternalFailed;
    }

    tjscalingfactor scalingFactor = {1, 1};
	int width, height, subsample, colorSpace;
	int result = tjDecompressHeader3(handle, data, size, &width, &height, &subsample, &colorSpace);
	if (result != 0)
	{
		CLOG_ERROR("Failed to decompress JPEG header: {}", tjGetErrorStr2(handle));
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
        CLOG_ERROR("Failed to set scaling factor: {}", tjGetErrorStr2(handle));
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
		result = tjDecompressToYUVPlanes(handle, data, size, &picture.GetData(), width, (int *)&picture.GetStride(), height, 0);
    }
    else
    {
        auto pitch = picture.GetStride(0);
        result = tjDecompress2(handle, data, size, picture.GetData(), width, pitch, height, TJPF_RGBX, 0);
    }

    if (result != 0)
    {
        CLOG_ERROR("Failed to decompress JPEG data: {}", tjGetErrorStr2(handle));
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
	info.quality = value;
}

void TurboJpegCodec::SetDesireSize(int value)
{
	desireSize = value;
}

}
}
