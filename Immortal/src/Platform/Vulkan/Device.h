#pragma once

#include <map>

#include "Framework/Device.h"

#include "vkcommon.h"
#include "ImmortalCore.h"

#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "CommandPool.h"
#include "FencePool.h"

namespace Immortal
{
namespace Vulkan
{
class Device : public Immortal::Device
{
public:
    Device() = default;

    Device(PhysicalDevice& physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions = {});

    ~Device();

    UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

    void CheckExtensionSupported();

    bool IsExtensionSupport(const char* extension);

    bool IsEnabled(const char* extension) const;

    Queue& FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);

    const Queue &SuitableGraphicsQueue();
    
public:
    virtual void *Handle() override
    {
        return handle;
    }

    virtual void *SuitableQueue() override
    {
        return SuitableGraphicsQueue().Handle();
    }

    template <class T>
    T& Get()
    {
        if constexpr (std::is_same_v<T, VkDevice>)
        {
            return handle;
        }
        if constexpr (std::is_same_v<T, PhysicalDevice>)
        {
            return physicalDevice;
        }
        if constexpr (std::is_same_v<T, Queue>)
        {
            return queues;
        }
    }

    void WaitIdle()
    {
        vkDeviceWaitIdle(handle);
    }

    VmaAllocator MemoryAllocator() const
    {
        return mMemoryAllocator;
    }

private:
    PhysicalDevice &physicalDevice;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkDevice handle{ VK_NULL_HANDLE };

    std::vector<VkExtensionProperties> mDeviceExtensions;

    std::vector<const char*> enabledExtensions{};

    VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

    std::vector<std::vector<Queue>> queues;
    Unique<CommandPool> commandPool;
    Unique<FencePool> fencePool;

    bool mHasGetMemoryRequirements = false;
    bool mHasDedicatedAllocation = false;
    bool mHasBufferDeviceAddressName = false;
};
}
}
