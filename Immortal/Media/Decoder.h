#pragma once

#include "Types.h"

namespace Media
{

class Decoder
{
public:
    Decoder(Type type, Format format) :
        type{ type },
        format{ format }
    {
    
    }

    virtual ~Decoder()
    {
    
    }

    virtual uint8_t *Data() const
    {
        return nullptr;
    }

private:
    Type type{ Type::Unspecifed };
    Format format{ Format::Unknown };
};

}
