#include "Raw.h"
#include "libraw/libraw.h"

#include <concepts>

namespace Immortal
{
namespace Vision
{

template <class T>
concept SampleType = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>;

template <SampleType T>
void ReadPixelsToPicture(T *dst, T *pixels, uint32_t width, uint32_t height, int channels)
{
    for (int i = 0; i < height; i++)
    {
        T *src = &pixels[i * width * channels];
        for (int j = 0; j < width; j++, dst += 4, src += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = T(-1);
        }
    }
}

RawCodec::RawCodec() :
    format{ Format::BayerLayerRGBG },
    bitDepth{ RawBitDepth::_16 }
{

}

RawCodec::~RawCodec()
{

}

CodecError RawCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.buffer;

    URef<LibRaw> processor = new LibRaw;
    processor->open_buffer(buffer.data(), buffer.size());
    processor->unpack();

    if (format == Format::BayerLayerRGBG)
    {
		auto &rawParams = processor->imgdata.rawdata.iparams;
		assert(rawParams.cdesc[0] == 'R' && rawParams.cdesc[1] == 'G' && rawParams.cdesc[2] == 'B' && rawParams.cdesc[3] == 'G');

		auto width  = processor->imgdata.rawdata.sizes.raw_width;
		auto height = processor->imgdata.rawdata.sizes.raw_height;
		picture = Picture{ width, height, Format::BayerLayerRGBG, true };

		memcpy(picture.GetData(), processor->imgdata.rawdata.raw_image, processor->imgdata.rawdata.sizes.raw_pitch * processor->imgdata.rawdata.sizes.raw_height);
    }
    else
    {
		processor->dcraw_process();
		processor->imgdata.params.output_bps = (int)bitDepth;

		int err = 0;
		libraw_processed_image_t *image = processor->dcraw_make_mem_image(&err);
        if (err)
        {
		    return CodecError::ExternalFailed;
        }

		if (bitDepth == RawBitDepth::_16)
		{
		    picture = Picture{ image->width, image->height, Format::RGBA16, true };
			ReadPixelsToPicture<uint16_t>((uint16_t *)picture.GetData(), (uint16_t *)image->data, image->width, image->height, image->colors);
		}
		else
		{
			picture = Picture{ image->width, image->height, Format::RGBA8, true };
			ReadPixelsToPicture<uint8_t>(picture.GetData(), image->data, image->width, image->height, image->colors);
		}
    }

    processor->recycle();
    return CodecError::Succeed;
}

}
}
