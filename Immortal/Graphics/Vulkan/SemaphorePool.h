#pragma once

#include "Common.h"
#include "Semaphore.h"
#include <list>

namespace Immortal
{
namespace Vulkan
{

class Device;
class SemaphorePool
{
public:
    VKCPP_SWAPPABLE(SemaphorePool)

public:
    SemaphorePool(Device *device = nullptr);

    ~SemaphorePool();

    Semaphore Allocate();

    TimelineSemaphore Allocate(uint64_t initialValue);

    void Release(Semaphore &semaphore);

    void Release(TimelineSemaphore &&semaphore);

public:
	void Swap(SemaphorePool &other)
    {
		std::swap(device, other.device);
		handles.swap(other.handles);
    }

protected:
    Semaphore InternalAllocate(const VkSemaphoreCreateInfo &createInfo);

protected:
    Device *device{ nullptr };

    std::queue<VkSemaphore> handles;
};

}
}
