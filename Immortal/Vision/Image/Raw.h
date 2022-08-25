#pragma once

#include "Vision/Interface/Codec.h"

namespace Immortal
{
namespace Vision
{

enum class RawBitDepth
{
    _8  = 8,
    _16 = 16,
};

class RawCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    RawCodec()
    {
        
    }

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    void SetBits(RawBitDepth v)
    {
        bitDepth = v;
    }

private:
    RawBitDepth bitDepth = RawBitDepth::_16;
};

}
}
