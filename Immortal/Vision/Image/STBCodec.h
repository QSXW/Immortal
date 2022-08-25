#pragma once

#include <cstdint>

#include "Vision/Interface/Codec.h"
#include "Vision/External/stb_image.h"

namespace Immortal
{
namespace Vision
{

class STBCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    STBCodec()
    {

    }

    virtual ~STBCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

    virtual CodecError Encode(const std::string& filepath, const Picture& picture) override;
};

}
}
