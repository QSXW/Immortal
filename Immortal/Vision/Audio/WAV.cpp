#include "WAV.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

CodecError WAVCodec::Decode(const CodedFrame &codedFrame)
{
    auto &buffer = codedFrame.buffer;

    memcpy(&header, buffer.data(), sizeof(header));

    auto headerSize = sizeof(header);
    if (*(uint32_t*)header.Subchunk2ID == *(uint32_t*)"LIST")
    {
        headerSize += header.Subchunk2Size;
    }

    picture = Picture{ (int)(buffer.size() - headerSize) / 2, 1, Format::BGRA8 };

    int16_t *src  = (int16_t *)(buffer.data() + headerSize);
    float *dst = (float *)picture.Data();
    for (int i = 0; i < picture.desc.width; i++)
    {
        dst[i] = src[i] / 32768.0;
    }

    return CodecError::Succeed;
}

}
}
