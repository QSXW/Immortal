#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

struct Timeline
{
    bool operator<(const Timeline &other) const
    {
        return value > other.value;
    }

    VkSemaphore semaphore;
    uint64_t value;
};

}
}
