#include "Webp.h"
#include <webp/decode.h>
#include <webp/encode.h>

namespace Immortal
{
namespace Vision
{

#if HAVE_WEBP
WebpCodec::WebpCodec() :
    Super{}
{

}

WebpCodec::~WebpCodec()
{

}

CodecError WebpCodec::Decode(const CodedFrame &codedFrame)
{
    int width, height, depth;
    const auto &buffer = codedFrame.GetBuffer();

    Format format = Format::RGBA8;
	uint8_t *data = WebPDecodeRGBA(buffer.data(), buffer.size(), &width, &height);
    if (!data)
    {
        return CodecError::CorruptedBitstream;
    }

	picture = Picture{uint32_t(width), uint32_t(height), format};
	picture.SetData(data);
	picture.SetStride(0, width * format.GetTexelSize());
	picture.SetRelease([](void *data) {
		WebPFree(data);
	});

    return CodecError::Success;
}

CodecError WebpCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    return CodecError::NotImplement;
}

#endif

}
}
