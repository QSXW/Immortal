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

CodecError RawCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.buffer;

    MonoRef<LibRaw> processor = new LibRaw;
    processor->open_buffer(buffer.data(), buffer.size());

    processor->unpack();
    processor->dcraw_process();
    processor->imgdata.params.output_bps = (int)bitDepth;

    int err = 0;;
    libraw_processed_image_t *image = processor->dcraw_make_mem_image(&err);
    if (err)
    {
        return CodecError::OutOfMemory;
    }
 
    if (bitDepth == RawBitDepth::_16)
    {
        picture = Picture{ image->width, image->height, Format::RGBA16 };
        ReadPixelsToPicture<uint16_t>((uint16_t *)picture.Data(), (uint16_t*)image->data, image->height, image->width, image->colors);
    }
    else
    {
        picture = Picture{ image->width, image->height, Format::RGBA8 };
        ReadPixelsToPicture<uint8_t>(picture.Data(), image->data, image->height, image->width, image->colors);
    }

    processor->recycle();
    return CodecError::Succeed;
}

}
}
