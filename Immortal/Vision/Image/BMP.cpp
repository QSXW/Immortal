#include "BMP.h"

#include <iostream>
#include <vector>

#include "FileSystem/Stream.h"

namespace Immortal
{ 
namespace Vision
{

CodecError BMPCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.buffer;
    memcpy(&identifer, buffer.data(), HeaderSize());

    if (bitsPerPixel != 24)
    {
        return CodecError::UnsupportFormat;
    }
    
    picture = Picture{ uint32_t(width), uint32_t(height), Format::BGRA8 };

    auto src = buffer.data() + offset;

    auto padding  = width & 3;
    auto linesize = width * picture.GetFormat().GetTexelSize();
    
    auto size = picture.GetFormat().GetTexelSize();

    uint8_t *ptr = picture.GetData() + size - linesize;

    for (int y = 0; y < height; y++)
    {
        uint8_t *dst = ptr;
        for (int x = 0; x < width; x++, dst += 4, src += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = ~0;
        }
        ptr = ptr - linesize;
        src += padding;
    }

    return CodecError::Succeed;
}

bool BMPCodec::Write(const std::string &filepath, int w, int h, int depth, uint8_t *data, int align)
{
    Stream stream{ filepath, Stream::Mode::Write };

    if (!stream.Writable())
    {
        return false;
    }

    auto padding  = w & 3;
    auto linesize = w * 3 + padding;
    auto dataSize = linesize * h;

    identifer       = 0x4d42;
    offset          = 54;
    fileSize        = dataSize + offset;
    informationSize = 0x28;
    width           = w;
    height          = h;
    planes          = 0x1;
    bitsPerPixel    = 0x18;
    imageSize       = dataSize;

    stream.Write(&identifer, offset);

    if (depth == 1)
    {
        for (int y = 0; y < height; y++, data += w + padding + align)
        {
            stream.Write(data, width, 3);
            stream.Skip(padding);
        }
    }
    else if (depth == 4)
    {
        std::vector<uint8_t> buf;
        buf.resize(linesize);

        size_t stride = (width * 4 + align);
        data += stride * height - stride;
        for (int y = 0; y < height; y++, data -= stride)
        {
            auto src = data;
            auto dst = buf.data();
            for (int x = 0; x < width; x++, dst += 3, src += 4)
            {
                dst[2] = src[0];
                dst[1] = src[1];
                dst[0] = src[2];
            }
            stream.Write(buf.data(), linesize, 1);
        }
    }
    else
    {
        for (int y = 0; y < height; y++, data += linesize + align)
        {
            stream.Write(data, linesize, 1);
            stream.Skip(padding);
        }
    }

    return true;
}

}
}
