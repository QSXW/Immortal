#pragma once

#include "Types.h"
#include "Format.h"

namespace Immortal
{
namespace Media
{

enum CodecError
{
    FailedToOpenFile = -1114,
    FailedToCallDecoder,
    CorruptedBitstream,
    Succeed = 0
};

class Decoder
{
public:
    Decoder()
    {

    }

    Decoder(Type type, Format format) :
        type{ type }
    {
        desc.Format = format;
    }

    virtual ~Decoder()
    {
    
    }

    virtual CodecError Decode(const std::vector<uint8_t> &buf)
    {
        return CodecError::FailedToCallDecoder;
    }

    virtual uint8_t *Data() const
    {
        return nullptr;
    }

    virtual void Swap(void *ptr)
    {
        ptr = nullptr;
    }

    uint32_t Width()
    {
        return desc.Width;
    }

    uint32_t Height()
    {
        return desc.Height;
    }

    const Description &Desc() const
    {
        return desc;
    }

protected:
    virtual void FillUpDescription()
    {
        desc.Spatial = (uint64_t)desc.Width * (uint64_t)desc.Height;
        desc.Size    = desc.Spatial * (uint64_t)desc.Depth * ((desc.Format == Format::RGBA32F) ? sizeof(float) :
                      ((desc.Format == Format::RGBA16) ? sizeof(uint16_t) : sizeof(uint8_t)));
    }

protected:
    Type type{ Type::Unspecifed };

    Description desc;
};

}
}