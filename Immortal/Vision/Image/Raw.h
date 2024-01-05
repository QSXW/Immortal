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
	RawCodec();

    virtual ~RawCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

public:
    void SetBits(RawBitDepth value)
    {
        bitDepth = value;
    }

protected:
    Format format;

    RawBitDepth bitDepth;
};

}
}
