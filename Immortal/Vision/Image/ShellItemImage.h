#pragma once

#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class ShellItemImage : public Interface::Codec
{
public:
	using Super = Interface::Codec;

public:
	ShellItemImage();

	virtual ~ShellItemImage() override;

	virtual CodecError Decode(const CodedFrame &codedFrame) override;
};

}
}
