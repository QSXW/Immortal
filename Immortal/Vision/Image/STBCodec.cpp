#include "STBCodec.h"

namespace Immortal
{
namespace Vision
{

STBCodec::STBCodec() :
    Super{}
{

}

STBCodec::~STBCodec()
{

}

CodecError STBCodec::Decode(const CodedFrame &codedFrame)
{
    int width, height, depth;
    const auto &buffer = codedFrame.GetBuffer();

    Format format = Format::RGBA8;
    uint8_t *data = nullptr;
	if (stbi_is_hdr_from_memory(buffer.data(), buffer.size()))
    {
        format = Format::RGBA32F;
        data = (uint8_t *)stbi_loadf_from_memory(
		    buffer.data(),
		    static_cast<int>(buffer.size()),
            &width,
            &height,
            &depth,
            4
        );
    }
    else
    {
        data = stbi_load_from_memory(
		    buffer.data(),
		    static_cast<int>(buffer.size()),
            &width,
            &height,
            &depth,
            4
        );
    }

    picture = Picture{ uint32_t(width), uint32_t(height), format };
    picture.SetData(data);
    if (!data)
    {
        return CodecError::CorruptedBitstream;
    }

    return CodecError::Succeed;
}

CodecError STBCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    return CodecError::NotImplement;
}

}
}
