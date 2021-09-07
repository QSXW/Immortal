#pragma once

#include "vkcommon.h"
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
    Wait();
    Reset();

    for (VkFence fence : handles)
    {
        IfNotNullThen<VkFence>(vkDestroyFence, device->Get<VkDevice>(), fence, nullptr);
    }
    handles.clear();
}

VkResult FencePool::Wait(UINT32 timeout) const
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    return vkWaitForFences(device->Get<VkDevice>(), activeCount, handles.data(), true, timeout);
}

VkResult FencePool::Reset()
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    activeCount = 0;
    return vkResetFences(device->Get<VkDevice>(), activeCount, handles.data());
}

VkFence FencePool::Request()
{
    if (activeCount < handles.size())
    {
        return handles.at(activeCount++);
    }

    VkFence fence{ VK_NULL_HANDLE };
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    Check(vkCreateFence(device->Get<VkDevice>(), &createInfo, nullptr, &fence));

    handles.emplace_back(fence);
    activeCount++;

    return handles.back();
}
}
}
