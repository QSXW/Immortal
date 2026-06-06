#include "PPM.h"

#include <iostream>
#include <vector>

#include "FileSystem/Stream.h"

namespace Immortal
{ 
namespace Vision
{

PPMCodec::PPMCodec()
{

}

PPMCodec::~PPMCodec()
{

}

#define PPM_HEADER "P3\n%d %d\n255\n"
CodecError PPMCodec::Decode(const CodedFrame &codedFrame)
{
	return CodecError::EndOfFile;
    int width, height;
    size_t n = sizeof(PPM_HEADER);
	auto ptr = (const char *)codedFrame.GetBuffer().data();
    sscanf(ptr, PPM_HEADER, &width, &height);

    uint8_t header[1024] = {};
    picture = Picture{ width, height, Format::RGBA8, true };
    for (int i = 0; i < height; i++)
    {
        auto dst = picture.GetData() + i * picture.GetStride(0);
        for (int j = 0; j < width; j++, dst += 4)
        {
            size_t size = n;
            dst[0] = 0xff;
            dst[1] = 0xff;
            dst[2] = 0xff;
            dst[3] = 0xff;
            n += size;
        }
    }

    return CodecError::Success;
}

CodecError PPMCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    auto width  = picture.GetWidth();
    auto height = picture.GetHeight();

    char header[1024] = {};
    auto n = sprintf(header, PPM_HEADER, width, height);

    /* if picture format is not rgba, color conversion is needed here. */
	std::vector<uint8_t> buffer;
    buffer.resize(n);
    std::copy(header, header + n, buffer.data());

    for (int i = 0; i < height; i++)
    {
        auto src = picture.GetData() + i * width * picture.GetFormat().GetTexelSize();
        for (int j = 0; j < width; j++, src += 4)
        {
            size_t size = buffer.size();
            n = sprintf(header, "%d %d %d\n", src[0], src[1], src[2]);
            buffer.resize(size + n);
            std::copy(header, header + n, buffer.data() + size);
        }
    }

    codedFrame = { std::move(buffer) };

    return CodecError::Success;
}

}
}
