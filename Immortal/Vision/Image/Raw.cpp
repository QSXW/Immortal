#include "Raw.h"
#include "libraw/libraw.h"

namespace Immortal
{ 
namespace Vision
{

CodecError RawCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.buffer;

    LibRaw processor;
    processor.open_buffer(buffer.data(), buffer.size());

    processor.unpack();
    processor.dcraw_process();
    processor.imgdata.params.output_bps = 16;

    int err = 0;;
    libraw_processed_image_t *image = processor.dcraw_make_mem_image(&err);
    if (err)
    {
        return CodecError::OutOfMemory;
    }

    picture = Picture{ image->width, image->height, Format::RGBA16 };

    auto dst = (uint16_t*)picture.Data();
    for (int i = 0; i < picture.desc.height; i++)
    {
        uint16_t *src = (uint16_t *)&image->data[i * image->width * image->colors * 2];
        for (int j = 0; j < picture.desc.width; j++, dst += 4, src += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = 0xffff;
        }
    }

    processor.recycle();
    return CodecError::Succeed;
}

}
}
