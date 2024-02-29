#pragma once

#include "Core.h"
#include "Codec.h"

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API VVCParser
{
public:
    CodecError Parse(const uint8_t *buffer, size_t size);

protected:
};

}
}
