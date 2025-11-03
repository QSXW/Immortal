#pragma once

#include "Vision/Codec.h"
#include <cstdint>

namespace Immortal
{
namespace Vision
{

class TIFFCodec : public Interface::Codec
{
public:
	using Super = Interface::Codec;

public:
	TIFFCodec();

	virtual ~TIFFCodec() override;

	virtual CodecError Decode(const CodedFrame &codedFrame) override;

	virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
};

}
}
