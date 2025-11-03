#include "WAV.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

CodecError WAVCodec::Decode(const CodedFrame &codedFrame)
{
	auto data = codedFrame.GetData();
	auto size = codedFrame.GetSize();

    memcpy(&header, data, sizeof(header));

    auto headerSize = sizeof(header);
    if (*(uint32_t*)header.Subchunk2ID == *(uint32_t*)"LIST")
    {
        headerSize += header.Subchunk2Size;
    }

    picture = Picture{ (int)(data - headerSize) >> 2, 1, Format::VECTOR2, true };

    int16_t *src  = (int16_t *)(data + headerSize);
    float *dst = (float *)picture.GetData();
    for (int i = 0; i < picture.GetWidth() * 2; i++)
    {
        dst[i] = src[i] / 32768.0;
    }

    return CodecError::Success;
}

}
}
