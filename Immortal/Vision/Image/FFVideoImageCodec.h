#pragma once

#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

// Decodes AVC, HEVC, VVC, and AV1 still-image containers, including FFmpeg tile grids.
class IMMORTAL_API FFVideoImageCodec : public Interface::Codec, public IClass
{
public:
    using Super = Interface::Codec;

public:
    FFVideoImageCodec();

    virtual ~FFVideoImageCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
};

}
}
