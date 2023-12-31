#include "SemaphorePool.h"

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

SemaphorePool::SemaphorePool(Device *device) :
    device{ device }
{

}

SemaphorePool::~SemaphorePool()
{
    while (!handles.empty())
    {
        device->DestroyAsync((VkSemaphore)handles.front());
        handles.pop();
    }
}

Semaphore SemaphorePool::Allocate()
{
    VkSemaphoreCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    return InternalAllocate(createInfo);
}

TimelineSemaphore SemaphorePool::Allocate(uint64_t initialValue)
{
    VkSemaphoreTypeCreateInfo timelineInfo{
        .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext         = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue  = initialValue,
    };

    VkSemaphoreCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timelineInfo,
        .flags = 0,
    };

    VkSemaphore semaphore = VK_NULL_HANDLE;
    Check(device->Create(&createInfo, &semaphore));

    return semaphore;
}

void SemaphorePool::Release(Semaphore &semaphore)
{
    handles.push(semaphore);
}

void SemaphorePool::Release(TimelineSemaphore &&semaphore)
{
    if (semaphore)
    {
        device->DestroyAsync((VkSemaphore)semaphore);
    }
}

Semaphore SemaphorePool::InternalAllocate(const VkSemaphoreCreateInfo &createInfo)
{
    VkSemaphore semaphore{VK_NULL_HANDLE};
    if (!handles.empty())
    {
        semaphore = handles.front();
        handles.pop();
    }
    else
    {
        Check(device->Create(&createInfo, &semaphore));
    }

    return semaphore;
}

}
}
