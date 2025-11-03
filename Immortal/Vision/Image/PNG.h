#pragma once

#include <cstdint>
#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class PNGCodec : public Interface::Codec, public IClass
{
public:
    using Super = Interface::Codec;

public:
	PNGCodec(const ImageEncodeInfo &info = {});

    virtual ~PNGCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

public:
	ImageEncodeInfo encodeInfo;
};

}
}
