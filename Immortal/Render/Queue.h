#pragma once

#include "Core.h"

namespace Immortal
{

class IMMORTAL_API Queue
{
public:
    enum class Type
    {
        Graphics     = BIT(0),
        Compute      = BIT(1),
        Transfer     = BIT(2),
        SparseBiding = BIT(3),
    };
};

using SuperQueue = Queue;

SL_DEFINE_BITWISE_OPERATION(Queue::Type, uint32_t)

}
