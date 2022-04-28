#pragma once

#include <cstdint>

#include "Media/Interface/Codec.h"
#include "Media/External/stb_image/stb_image.h"

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

    virtual uint8_t *Data() const override;

    virtual void Swap(void *ptr) override;

private:
    stbi_uc *data = nullptr;
};

}
}
