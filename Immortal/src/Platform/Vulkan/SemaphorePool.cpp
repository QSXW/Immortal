#pragma once

#include "Common.h"
#include "Device.h"
#include "SemaphorePool.h"

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
        IfNotNullThen(vkDestroySemaphore, *device, h, nullptr);
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

    Check(vkCreateSemaphore(*device, &createInfo, nullptr, &semaphore));

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
