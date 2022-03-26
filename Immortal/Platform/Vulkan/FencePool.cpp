#include "FencePool.h"
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

VkResult FencePool::Wait(uint32_t timeout) const
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    return device->Wait(handles.data(), activeCount, true, timeout);
}

VkResult FencePool::Reset()
{
    if (activeCount < 1 || handles.empty())
    {
        return VK_SUCCESS;
    }
    activeCount = 0;
    return device->Reset(handles.data(), handles.size());
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
        Check(device->Create(&createInfo, &fence));

        handles.emplace_back(fence);
        activeCount++;
    }

    return fence;
}

void FencePool::Discard(VkFence *pFence)
{
    device->Reset(pFence, 1);
    pending.push(*pFence);
    activeCount--;
    *pFence = nullptr;
}

}
}
