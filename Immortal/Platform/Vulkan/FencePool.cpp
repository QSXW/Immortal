#pragma once

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

FencePool::FencePool(Device *device) :
    device{ device }
{
        
}

FencePool::~FencePool()
{
    for (VkFence fence : handles)
    {
        device->DestroyAsync(fence);
    }
    handles.clear();
}

VkResult FencePool::Wait(UINT32 timeout) const
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    return vkWaitForFences(*device, activeCount, handles.data(), true, timeout);
}

VkResult FencePool::Reset()
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    activeCount = 0;
    return vkResetFences(*device, activeCount, handles.data());
}

VkFence FencePool::Request()
{
    VkFence fence{ VK_NULL_HANDLE };
    if (!pending.empty())
    {
        fence = pending.front();
        pending.pop();
    }
    else 
    {
        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        Check(vkCreateFence(*device, &createInfo, nullptr, &fence));

        handles.emplace_back(fence);
        activeCount++;
    }

    return fence;
}

void FencePool::Discard(VkFence *pFence)
{
    vkResetFences(*device, 1, pFence);
    pending.push(*pFence);
    activeCount--;
    *pFence = nullptr;
}

}
}
