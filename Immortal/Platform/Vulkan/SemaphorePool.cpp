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
    Reset();
    for (auto &h : handles)
    {
        device->DestroyAsync(h);
    }
    handles.clear();
}

VkSemaphore SemaphorePool::Request()
{
    if (activeCount < handles.size())
    {
        return handles.at(activeCount++);
    }

    VkSemaphore semaphore{ VK_NULL_HANDLE };
    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    Check(device->Create(&createInfo, &semaphore));

    handles.push_back(semaphore);
    activeCount++;

    return semaphore;
}

void SemaphorePool::Reset()
{
    activeCount = 0;
}

}
}
