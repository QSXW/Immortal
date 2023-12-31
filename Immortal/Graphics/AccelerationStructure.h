#pragma once

#include "Core.h"

namespace Immortal
{

class IMMORTAL_API AccelerationStructure
{
public:
    AccelerationStructure() = default;

    virtual ~AccelerationStructure() = default;
};

using SuperAccelerationStructure = AccelerationStructure;

}
