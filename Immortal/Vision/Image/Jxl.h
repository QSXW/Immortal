#pragma once

#include <cstdint>
#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class JxlCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

#if HAVE_JXL
public:
	JxlCodec();

    virtual ~JxlCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
#endif
};

}
}
