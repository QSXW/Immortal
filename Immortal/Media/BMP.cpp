#include "BMP.h"

#include <iostream>
#include <vector>
#include "Stream.h"
#include "ColorSpace.h"

namespace Immortal
{ 
namespace Media
{

bool BMPCodec::Read(const std::string &filename, bool alpha)
{
    Stream stream{ filename.c_str(), Stream::Mode::Read };

    if (!stream.Readable())
    {
        return false;
    }
    stream.Read(&identifer, HeaderSize());

    if (bitsPerPixel != 24)
    {
        return false;
    }

    if (alpha)
    {
        depth = 4;
    }

    stream.Locate(offset);

    auto padding  = width & 3;
    auto linesize = width * 3;
    
    auto size = (linesize + /* alpha */width) * height;
    data.reset(new uint8_t [size]);
    if (!data)
    {
        return false;
    }

    uint8_t *ptr = data.get() + size - linesize - width;
    if (alpha)
    {
        std::vector<uint8_t> buffer{};
        buffer.resize(linesize);
        for (int y = 0; y < height; y++)
        {
            uint8_t *src = buffer.data();
            stream.Read(src, linesize);
            stream.Skip(padding);
            uint8_t *dst = ptr;
            for (int x = 0; x < width; x++, dst += 4, src += 3)
            {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = ~0;
            }
            ptr = ptr - linesize - width;
        }
    }
    else
    {
        for (int y = 0; y < height; y++, ptr -= linesize)
        {
            stream.Read(ptr, linesize);
            stream.Skip(padding);
        }
    }

    desc.Depth  = alpha ? 4 : 3;
    FillUpDescription();

    return true;
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

void BMPCodec::FillUpDescription()
{
    desc.Width  = width;
    desc.Height = height;
    Decoder::FillUpDescription();
}

}
}
