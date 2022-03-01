#pragma once

#include "Format.h"
#include "Media/Types.h"
#include "Media/Common/Error.h"

namespace Immortal
{
namespace Vision
{
namespace Interface
{

class Codec
{
public:
    Codec()
    {

    }

    Codec(Type type, Format format) :
        type{ type }
    {
        desc.format = format;
    }

    virtual ~Codec()
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

    const Description &Desc() const
    {
        return desc;
    }

protected:
    Type type{ Type::Unspecifed };

    Description desc;
};

}
}
}
