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

class Swapchain;
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

    VkResult CreateBuffer(const VkBufferCreateInfo *pCreateInfo, VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
    {
        return vkCreateBuffer(handle, pCreateInfo, pAllocator, pBuffer);
    }

    void GetRequirements(VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements)
    {
        vkGetBufferMemoryRequirements(handle, buffer, pMemoryRequirements);
    }

    void GetRequirements(VkImage image, VkMemoryRequirements *pMemoryRequirements)
    {
        vkGetImageMemoryRequirements(handle, image, pMemoryRequirements);
    }

    uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memoryTypeFound = nullptr);

    VkResult AllocateMemory(const VkMemoryAllocateInfo *pAllocateInfo, VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
    {
        return vkAllocateMemory(handle, pAllocateInfo, pAllocator, pMemory);
    }

    VkResult BindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
    {
        return vkBindBufferMemory(handle, buffer, memory, memoryOffset);
    }

    VkResult BindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
    {
        return vkBindImageMemory(handle, image, memory, memoryOffset);
    }

    VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
    {
        return vkMapMemory(handle, memory, offset, size, flags, ppData);
    }

    void UnmapMemory(VkDeviceMemory memory)
    {
        vkUnmapMemory(handle, memory);
    }

    VkResult CreateImage(const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage)
    {
        return vkCreateImage(handle, pCreateInfo, pAllocator, pImage);
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

    VkResult GetSurfaceCapabilities(Surface &surface, VkSurfaceCapabilitiesKHR *properties)
    {
        return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.Handle(), surface, properties);
    }

    CommandBuffer *Request(Level level)
    {
        return commandPool->RequestBuffer(level);
    }

    void Discard(CommandBuffer *commandBuffer)
    {
        commandPool->DiscardBuffer(commandBuffer);
    }

private:
    PhysicalDevice &physicalDevice;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkDevice handle{ VK_NULL_HANDLE };

    std::vector<VkExtensionProperties> deviceExtensions;

    std::vector<const char *> enabledExtensions{};

    VmaAllocator memoryAllocator{ VK_NULL_HANDLE };

    std::vector<std::vector<Queue>> queues;

    std::unique_ptr<CommandPool> commandPool;

    std::unique_ptr<FencePool> fencePool;

    bool hasGetMemoryRequirements{ false };
    bool hasDedicatedAllocation{ false };
    bool hasBufferDeviceAddressName{ false };
};
}
}
