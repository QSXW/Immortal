#include "STBCodec.h"

namespace Immortal
{
namespace Vision
{

STBCodec::~STBCodec()
{

}

CodecError STBCodec::Decode(const CodedFrame &codedFrame)
{
    int width, height, depth;
    const auto &buf = codedFrame.buffer;

    Format format = Format::RGBA8;
    uint8_t *data = nullptr;
    if (stbi_is_hdr_from_memory(buf.data(), codedFrame.buffer.size()))
    {
        format = Format::RGBA32F;
        data = (uint8_t *)stbi_loadf_from_memory(
            buf.data(),
            static_cast<int>(buf.size()),
            &width,
            &height,
            &depth,
            4
        );
    }
    else
    {
        data = stbi_load_from_memory(
            buf.data(),
            static_cast<int>(buf.size()),
            &width,
            &height,
            &depth,
            4
        );
    }

    picture = Picture{ width, height, format, false };
    picture.data.reset(data);
    if (!data)
    {
        return CodecError::CorruptedBitstream;
    }

    return CodecError::Succeed;
}

CodecError STBCodec::Encode(const Picture & picture, CodedFrame & codedFrame)
{
    return CodecError();
}

}
}
