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
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    return InternalRequest(createInfo);
}

VkSemaphore SemaphorePool::Request(uint64_t initialValue)
{
    VkSemaphoreTypeCreateInfo timelineInfo{};
    timelineInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineInfo.pNext         = nullptr;
    timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineInfo.initialValue  = initialValue;

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &timelineInfo;
    createInfo.flags = 0;

    return InternalRequest(createInfo);
}

void SemaphorePool::Reset()
{
    activeCount = 0;
}

VkSemaphore SemaphorePool::InternalRequest(const VkSemaphoreCreateInfo &createInfo)
{
    if (activeCount < handles.size())
    {
        return handles.at(activeCount++);
    }

    VkSemaphore semaphore{ VK_NULL_HANDLE };
    Check(device->Create(&createInfo, &semaphore));

    handles.push_back(semaphore);
    activeCount++;

    return semaphore;
}

}
}
