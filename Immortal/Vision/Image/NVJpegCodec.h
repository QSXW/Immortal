#pragma once

#include <cstdint>

#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class NVJpegCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

#if HAVE_NVJPEG
public:
	NVJpegCodec();

    virtual ~NVJpegCodec() override;

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;
#endif

    CodecError Decode(const uint8_t *data, size_t size);
};

}
}
