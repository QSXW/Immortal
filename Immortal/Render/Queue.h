#pragma once

#include "Core.h"

namespace Immortal
{

class IMMORTAL_API Queue
{
public:
    enum class Type
    {
        None          = BIT(31),
        Graphics      = BIT(0),
        Compute       = BIT(1),
        Transfer      = BIT(2),
        SparseBiding  = BIT(3),
        VideoDecoding = BIT(5),
        VideoEncoding = BIT(6)
    };
};

using SuperQueue = Queue;

SL_ENABLE_BITWISE_OPERATOR(Queue::Type)

}
