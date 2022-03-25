#include <iostream>
#include "Immortal.h"
#include "Media/Stream.h"
#include "Media/Interface/Codec.h"
#include "Media/Video/HEVC.h"
#include "Media/Common/BitTracker.h"
#include "Media/Image/JPEG.h"

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
