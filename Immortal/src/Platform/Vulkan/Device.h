#pragma once
#include "ImmortalCore.h"

#include <map>
#include <unordered_map>

#include "Framework/Device.h"

#include "Common.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "CommandPool.h"
#include "FencePool.h"

namespace Immortal
{
namespace Vulkan
{
class Device : public SuperDevice
{
public:
    Device() = default;

    Device(PhysicalDevice &physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});

    ~Device();

    UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

    void CheckExtensionSupported();

    bool IsExtensionSupport(const char *extension);

    bool IsEnabled(const char *extension) const;

    Queue &FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);

    Queue &SuitableGraphicsQueue();

    Queue *SuitableGraphicsQueuePtr()
    {
        auto &queue = SuitableGraphicsQueue();
        return &queue;
    }

    VkFormat DepthFormat(bool depthOnly = false)
    {
        return physicalDevice.DepthFormat(depthOnly);
    }

    VkFence RequestFence()
    {
        return fencePool->Request();
    }

public:
    VkDevice Handle()
    {
        return handle;
    }

    template <class T>
    T &Get()
    {
        if constexpr (typeof<T, VkDevice>())
        {
            return handle;
        }
        if constexpr (typeof<T, PhysicalDevice>())
        {
            return physicalDevice;
        }
        if constexpr (typeof<T, Queue>())
        {
            return queues;
        }
        if constexpr (typeof<T, VkPhysicalDevice>())
        {
            return physicalDevice.Handle();
        }
        if constexpr (typeof<T, CommandPool>())
        {
            return *commandPool;
        }
    }

    VkResult Wait()
    {
        return vkDeviceWaitIdle(handle);
    }

    VmaAllocator MemoryAllocator() const
    {
        return memoryAllocator;
    }

private:
    PhysicalDevice &physicalDevice;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkDevice handle{ VK_NULL_HANDLE };

    std::vector<VkExtensionProperties> deviceExtensions;

    std::vector<const char *> enabledExtensions{};

    VmaAllocator memoryAllocator{ VK_NULL_HANDLE };

    std::vector<std::vector<Queue>> queues;

    Unique<CommandPool> commandPool;

    Unique<FencePool> fencePool;

    bool hasGetMemoryRequirements{ false };
    bool hasDedicatedAllocation{ false };
    bool hasBufferDeviceAddressName{ false };
};
}
}
