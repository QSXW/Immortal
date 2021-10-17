#include "BMP.h"

#include <iostream>
#include "Stream.h"

namespace Media
{

Error BMPDecoder::Read(const std::string &filename)
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

    data.reset(new uint8_t [width * height * 3]);

    if (!data)
    {
        return Error::OUT_OF_MEMORY;
    }
    stream.Locate(offset);

    auto *ptr     = data.get();
    auto padding  = width & 3;
    auto linesize = width * depth;

    for (int y = 0; y < height; y++, ptr += linesize)
    {
        stream.Read(ptr, linesize);
        stream.Skip(padding);
    }

    return Error::SUCCEED;
}

}
