#pragma once

#include "Common.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Semaphore : public Handle<VkSemaphore>
{
public:
    Semaphore(VkSemaphore handle = VK_NULL_HANDLE) :
        Handle{ handle }
    {

    }

    Semaphore(const Semaphore &other) :
        Handle{ other.handle }
    {

    }

    Semaphore &operator=(const Semaphore &other)
    {
        handle = other.handle;
        return *this;
    }
};

class TimelineSemaphore : public Semaphore
{
public:
    VKCPP_SWAPPABLE(TimelineSemaphore)

public:
    TimelineSemaphore(VkSemaphore semaphore = VK_NULL_HANDLE, uint64_t value = 0) :
        Semaphore{ semaphore },
        value{ value }
    {

    }

    void Swap(TimelineSemaphore &other)
    {
        Handle::Swap((Handle &)other);
        std::swap(value, other.value);
    }

    uint64_t value;
};

}
}
