#include <iostream>

#include "Immortal.h"
#include "FileSystem/Stream.h"
#include "Vision/Image/Image.h"
#include "Vision/Video/Video.h"

using namespace Immortal;

int main()
{
    LOG::Setup();

    {
        std::string path = "1920x800_25fps.265";
        Stream stream{ path, Stream::Mode::Read };
        THROWIF(!stream.Readable(), "Unable to open file");

        std::vector<uint8_t> buffer;
        buffer.reserve(SLALIGN(stream.Size(), 64));
        buffer.resize(stream.Size());
        stream.Read(buffer);

        Vision::HEVCCodec decoder;
        decoder.Decode(buffer);
    }

    return 0;
}
