#include "STBCodec.h"
#include "Vision/External/stb_image.h"

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
    
    auto data = codedFrame.GetData();
	auto size = codedFrame.GetSize();

    Format format = Format::RGBA8;
    uint8_t *buf = nullptr;
	if (stbi_is_hdr_from_memory(data, size))
    {
        format = Format::R32G32B32A32_SFLOAT;
		buf = (uint8_t *) stbi_loadf_from_memory(
		    data,
		    static_cast<int>(size),
            &width,
            &height,
            &depth,
            4
        );
    }
    else
    {
		buf = stbi_load_from_memory(
		    data,
		    static_cast<int>(size),
            &width,
            &height,
            &depth,
            4
        );
    }

	if (!buf)
	{
		return CodecError::CorruptedBitstream;
	}

    picture = Picture{ uint32_t(width), uint32_t(height), format };
	picture.SetData(buf);
	picture.SetRelease([](void *buf) {
		stbi_image_free(buf);
	});

    return CodecError::Success;
}

CodecError STBCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    return CodecError::NotImplement;
}

}
}
