#pragma once

#include "vkcommon.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
    FencePool::FencePool(Device &device) :
        device{ device }
    {
        
    }

    FencePool::~FencePool()
    {
        Wait();
        Reset();

        for (VkFence fence : fences)
        {
            IfNotNullThen<VkFence>(vkDestroyFence, device.Handle(), fence, nullptr);
        }
        fences.clear();
    }

    VkResult FencePool::Wait(UINT32 timeout) const
    {
        if (activeFenceCount < 1 || fences.empty())
        {
            return VK_SUCCESS;
        }
        return vkWaitForFences(device.Handle(), activeFenceCount, fences.data(), true, timeout);
    }

    VkResult FencePool::Reset()
    {
        if (activeFenceCount < 1 || fences.empty())
        {
            return VK_SUCCESS;
        }
        activeFenceCount = 0;
        return vkResetFences(device.Handle(), activeFenceCount, fences.data());
    }
    VkFence FencePool::RequestFence()
    {
        if (activeFenceCount < fences.size())
        {
            return fences.at(activeFenceCount++);
        }

        VkFence fence{ VK_NULL_HANDLE };
        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        Check(vkCreateFence(device.Handle(), &createInfo, nullptr, &fence));

        fences.emplace_back(fence);
        activeFenceCount++;

        return fences.back();
    }
}
}
