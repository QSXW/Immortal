#pragma once

#include "Vision/Interface/Codec.h"

namespace Immortal
{
namespace Vision
{

class RawCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    RawCodec()
    {
        
    }

    virtual CodecError Decode(const CodedFrame &codedFrame) override;
};

}
}
