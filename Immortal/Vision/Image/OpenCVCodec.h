#pragma once

#include <cstdint>

#include "Config.h"
#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class OpenCVCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    OpenCVCodec()
    {
        
    }

#if HAVE_OPENCV
    virtual ~OpenCVCodec();

    virtual CodecError Decode(const CodedFrame &codedFrame) override;

    virtual CodecError Encode(const Picture &picture, CodedFrame &codedFrame) override;

    virtual void Swap(void *ptr);
#endif
};

}
}
