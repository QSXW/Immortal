#include "BMP.h"

#include <iostream>
#include "Stream.h"

namespace Media
{

Error BMPDecoder::Read(const std::string &filename, bool alpha)
{
    Stream stream{ filename.c_str(), Stream::Mode::Read };

    if (!stream.Readable())
    {
        return Error::UNABLE_TO_OEPN_FILE;
    }
    stream.Read(&identifer, HeaderSize());

    if (bitsPerPixel != 24)
    {
        return Error::UNSUPPORT_FORMAT;
    }

    if (alpha = true)
    {
        depth = 4;
    }
    data.reset(new uint8_t [width * height * depth]);

    if (!data)
    {
        return Error::OUT_OF_MEMORY;
    }
    stream.Locate(offset);

    auto *ptr     = data.get();
    auto padding  = width & 3;
    auto linesize = width * depth;
    
    if (alpha)
    {
        for (int y = 0; y < height; y++, ptr += linesize)
        {
            stream.Read(ptr, linesize);
            stream.Skip(padding);
        }
    }
    else
    {
        std::unique_ptr<uint8_t> buffer{ new uint8_t[linesize] };
        for (int y = 0; y < height; y++, ptr += linesize)
        {
            stream.Read(buffer.get(), linesize);
            stream.Skip(padding);

            auto src = buffer.get();
            for (int x = 0; x < width; x++, ptr += 4, src += 3)
            {
                ptr[0] = src[0];
                ptr[1] = src[1];
                ptr[2] = src[2];
                ptr[3] = ~0;
            }
        }
    }

    return Error::SUCCEED;
}

Error BMPDecoder::Write(const std::string &filepath, int h, int w, int depth, uint8_t *data)
{
    Stream stream{ filepath, Stream::Mode::Write };

    if (!stream.Writable())
    {
        return Error::UNABLE_TO_OEPN_FILE;
    }

    auto padding  = w & 3;
    auto linesize = (w + padding) * 3;
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
        for (int y = 0; y < height; y++, data += w + padding)
        {
            stream.Write(data, width, 3);
            stream.Skip(padding);
        }
    }
    else
    {
        for (int y = 0; y < height; y++, data += linesize)
        {
            stream.Write(data, linesize, 1);
            stream.Skip(padding);
        }
    }

    return Error::SUCCEED;
}

}
