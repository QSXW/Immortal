#pragma once

#include <cstdint>
#include "Render/Descriptor.h"

namespace Immortal
{
namespace OpenGL
{

struct Descriptor : public SuperDescriptor
{
    using Super = SuperDescriptor;

    Descriptor() :
        handle{ 0 }
    {

    }

    Descriptor(uint32_t handle) :
        handle{ handle }
    {

    }

    uint32_t handle;
};

}
}
