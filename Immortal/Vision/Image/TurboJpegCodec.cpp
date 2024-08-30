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

		default:
			return TJSAMP_UNKNOWN;
	}
}

TurboJpegCodec::TurboJpegCodec(bool isOutputYUV) :
    Super{},
    numerator{ 1 },
	denominator{ 1 },
    isOutputYUV{ isOutputYUV },
    quality{ 90 }
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

	if (picture.GetFormat().IsType(Format::YUV))
	{	
		int strides[4] = {};
		uint8_t *planes[4] = {};
		for (size_t i = 0; picture.GetData(i); i++)
		{
			strides[i] = picture.GetStride(i);
			planes[i]  = picture.GetData(i);
		}

		tjCompressFromYUVPlanes(handle, (const unsigned char **)planes, picture.GetWidth(), strides, picture.GetHeight(), sampling, &compressedRef.data, &compressedRef.size, quality, 0);
		codedFrame = CodedFrame{ compressedRef.data, compressedRef.size };
	}

	tjDestroy(handle);
    return CodecError::Success;
}
#endif

CodecError TurboJpegCodec::Decode(const uint8_t *data, size_t size)
{
#if HAVE_TURBOJPEG
    tjhandle handle = {};

    handle = tjInitDecompress();

    int width, height, subsample, colorSpace;
	tjDecompressHeader3(handle, data, size, &width, &height, &subsample, &colorSpace);

    tjscalingfactor scalingFactor = {
        .num   = numerator,
        .denom = denominator
    };

	width  = TJSCALED(width, scalingFactor );
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
	
    picture = Picture{ width, height, format, true };

	if (isOutputYUV)
	{
		//jpeg_decompress_struct cinfo = {};
		//jpeg_error_mgr jerr = {};
		//cinfo.err = jpeg_std_error(&jerr);

		//jerr.error_exit = [](j_common_ptr cinfo) {
		//	char pszErr[1024];
		//	(cinfo->err->format_message)(cinfo, pszErr);
		//	throw std::runtime_error(pszErr);
		//};

		//jpeg_create_decompress(&cinfo);
		//jpeg_mem_src(&cinfo, data, size);

		//(void) jpeg_read_header(&cinfo, TRUE);

		//jvirt_barray_ptr *coefficients = jpeg_read_coefficients(&cinfo);

		//for (int i = 0; i < cinfo.num_components; i++)
		//{
		//	auto w = cinfo.comp_info[i].width_in_blocks;
		//	auto h = cinfo.comp_info[i].height_in_blocks;

		//	auto compptr = &cinfo.comp_info[i];
		//	for (JDIMENSION blk_y = 0; blk_y < compptr->height_in_blocks; blk_y += compptr->v_samp_factor)
		//	{
		//		auto buffer = (*cinfo.mem->access_virt_barray)((j_common_ptr) &cinfo, coefficients[i], blk_y, (JDIMENSION)compptr->v_samp_factor, TRUE);
		//		for (JDIMENSION offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
		//		{
		//			auto block = buffer[offset_y];
		//			memcpy(picture.GetData(i), &block[0], compptr->width_in_blocks * DCTSIZE2);
		//		}
		//	}
		//}

		tjDecompressToYUV2(handle, data, size, picture.GetData(), width, 8, height, 0);
	}
	else
	{
		auto pitch = picture.GetStride(0);
		tjDecompress2(handle, data, size, picture.GetData(), width, pitch, height, TJPF_RGBA, 0);
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

}
}
