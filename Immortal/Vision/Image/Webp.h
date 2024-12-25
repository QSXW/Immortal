#pragma once

#include <cstdint>
#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class WebpCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

#if HAVE_WEBP
public:
	WebpCodec();

    virtual ~WebpCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
#endif
};

}
}
