#pragma once

#include "Codec.h"

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API RawSpeedCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
	RawSpeedCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;
};

}
}
