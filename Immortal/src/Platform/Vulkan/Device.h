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
#include "DescriptorPool.h"

namespace Immortal
{
namespace Vulkan
{

class Swapchain;
class Device : public SuperDevice
{
public:
    using UploadProcess = void (*)(CommandBuffer *);

public:
    Device() = default;

    Device(PhysicalDevice &physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});

    ~Device();

    UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

    Queue &FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex);

    Queue &SuitableGraphicsQueue();

public:
    Queue *SuitableGraphicsQueuePtr()
    {
        auto &queue = SuitableGraphicsQueue();
        return &queue;
    }

    VkFormat DepthFormat(bool depthOnly = false)
    {
        return physicalDevice.DepthFormat(depthOnly);
    }

    bool Device::IsEnabled(const std::string &extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(), [extension](const std::string &enabledExtension)
        {
            return Vulkan::Equals(extension.c_str(), enabledExtension.c_str());
        }) != enabledExtensions.end();
    }

    bool Device::IsExtensionSupport(const std::string &extension) const
    {
        return availableExtensions.find(extension) != availableExtensions.end();
    }

public:
    VkFence RequestFence()
    {
        return fencePool->Request();
    }

    VkResult Create(const VkBufferCreateInfo *pCreateInfo, VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
    {
        return vkCreateBuffer(handle, pCreateInfo, pAllocator, pBuffer);
    }

#define DEFINE_DESTORY_VK_OBJECT(T) \
    void Destory(Vk##T object, const VkAllocationCallbacks* pAllocator = nullptr) \
    { \
        IfNotNullThen(vkDestroy##T, handle, object, pAllocator); \
    }

    DEFINE_DESTORY_VK_OBJECT(Image)
    DEFINE_DESTORY_VK_OBJECT(ImageView)
    DEFINE_DESTORY_VK_OBJECT(Buffer)
    DEFINE_DESTORY_VK_OBJECT(DescriptorSetLayout)
    DEFINE_DESTORY_VK_OBJECT(PipelineLayout)
    DEFINE_DESTORY_VK_OBJECT(ShaderModule)

    VkResult CreatePipelineLayout(const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout)
    {
        return vkCreatePipelineLayout(handle, pCreateInfo, pAllocator, pPipelineLayout);
    }

    VkResult CreateSampler()
    {

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

    void FreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator = nullptr)
    {
        vkFreeMemory(handle, memory, pAllocator);
    }

    VkResult CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout)
    {
        return vkCreateDescriptorSetLayout(handle, pCreateInfo, pAllocator, pSetLayout);
    }

    VkResult AllocateDescriptorSet(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSets)
    {
        return descriptorPool->Allocate(pDescriptorSetLayout, pDescriptorSets);
    }

    void FreeDescriptorSet(VkDescriptorSet *pDescriptorSets, uint32_t size = 1)
    {
        descriptorPool->Free(pDescriptorSets, 1);
    }

    void UpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies)
    {
        vkUpdateDescriptorSets(handle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
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
    VkDevice &Handle()
    {
        return handle;
    }

    operator VkDevice&()
    {
        return handle;
    }

    template <class T>
    T &Get()
    {
        if constexpr (is_same<T, VkDevice>())
        {
            return handle;
        }
        if constexpr (is_same<T, PhysicalDevice>())
        {
            return physicalDevice;
        }
        if constexpr (is_same<T, Queue>())
        {
            return queues;
        }
        if constexpr (is_same<T, VkPhysicalDevice>())
        {
            return physicalDevice.Handle();
        }
        if constexpr (is_same<T, CommandPool>())
        {
            return *commandPool;
        }
    }

    VkResult Wait()
    {
        return vkDeviceWaitIdle(handle);
    }

    VkResult Wait(VkFence *pFences, uint32_t fenceCount = 1, VkBool32 waitAll = VK_TRUE, uint64_t timeout = std::numeric_limits<uint64_t>::max())
    {
        return vkWaitForFences(handle, fenceCount, pFences, waitAll, timeout);
    }

    VkResult Reset(VkFence *pFence, uint32_t fenceCount = 1)
    {
        return vkResetFences(handle, 1, pFence);
    }

    void WaitAndReset(VkFence *pFence, uint32_t fenceCount = 1)
    {
        Check(Wait(pFence, fenceCount));
        Check(Reset(pFence, fenceCount));
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

    CommandBuffer *BeginUpload()
    {
        auto *copyCmd = commandPool->RequestBuffer(Level::Primary);
        copyCmd->Begin();
        return copyCmd;
    }

    void EndUpload(CommandBuffer *copyCmd)
    {
        copyCmd->End();

        auto &queue = FindQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &copyCmd->Handle();

        auto fence = fencePool->Request();

        queue.Submit(submitInfo, fence);
        Check(Wait(&fence, 1, VK_TRUE, FencePool::Timeout));

        fencePool->Discard(&fence);
        commandPool->DiscardBuffer(copyCmd);
    }

    template <class T>
    void Upload(T &&process)
    {
        auto *copyCmd = BeginUpload();
        process(copyCmd);
        EndUpload(copyCmd);
    }

    void Discard(CommandBuffer *commandBuffer)
    {
        commandPool->DiscardBuffer(commandBuffer);
    }

private:
    PhysicalDevice &physicalDevice;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkDevice handle{ VK_NULL_HANDLE };

    std::unordered_set<std::string> availableExtensions;

    std::vector<const char *> enabledExtensions;

    VmaAllocator memoryAllocator{ VK_NULL_HANDLE };

    std::vector<std::vector<Queue>> queues;

    std::unique_ptr<CommandPool> commandPool;

    std::unique_ptr<FencePool> fencePool;

    std::unique_ptr<DescriptorPool> descriptorPool;
};
}
}
