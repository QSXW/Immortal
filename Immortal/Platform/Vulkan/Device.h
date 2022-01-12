#pragma once
#include "Core.h"

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

    static inline Device *That = nullptr;

    static inline constexpr bool isLogNeed = false;

    void EnableGlobal()
    {
        That = this;
    }

public:
    Device() = default;

    Device(PhysicalDevice &physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});

    ~Device();

    uint32_t QueueFailyIndex(VkQueueFlagBits queueFlag);

    Queue &FindQueueByType(Queue::Type type, uint32_t queueIndex);

    Queue &SuitableGraphicsQueue();

    uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memoryTypeFound = nullptr);

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
            return Equals(extension.c_str(), enabledExtension.c_str());
        }) != enabledExtensions.end();
    }

    bool Device::IsExtensionSupport(const std::string &extension) const
    {
        return availableExtensions.find(extension) != availableExtensions.end();
    }

public:
#define DEFINE_CREATE_VK_OBJECT(T) \
    VkResult Create(const Vk##T##CreateInfo *pCreateInfo, VkAllocationCallbacks *pAllocator, Vk##T *pObject) \
    { \
        return vkCreate##T(handle, pCreateInfo, pAllocator, pObject); \
    }

    DEFINE_CREATE_VK_OBJECT(Buffer)
    DEFINE_CREATE_VK_OBJECT(DescriptorSetLayout)
    DEFINE_CREATE_VK_OBJECT(Framebuffer)
    DEFINE_CREATE_VK_OBJECT(Image)
    DEFINE_CREATE_VK_OBJECT(PipelineLayout)
    DEFINE_CREATE_VK_OBJECT(RenderPass)
    DEFINE_CREATE_VK_OBJECT(Sampler)
    DEFINE_CREATE_VK_OBJECT(ShaderModule)

#define DEFINE_DESTORY_VK_OBJECT(T) \
    void Destory(Vk##T object, const VkAllocationCallbacks* pAllocator = nullptr) \
    { \
        IfNotNullThen(vkDestroy##T, handle, object, pAllocator); \
    }

    DEFINE_DESTORY_VK_OBJECT(Buffer)
    DEFINE_DESTORY_VK_OBJECT(DescriptorSetLayout)
    DEFINE_DESTORY_VK_OBJECT(Framebuffer)
    DEFINE_DESTORY_VK_OBJECT(Image)
    DEFINE_DESTORY_VK_OBJECT(ImageView)
    DEFINE_DESTORY_VK_OBJECT(Pipeline)
    DEFINE_DESTORY_VK_OBJECT(PipelineLayout)
    DEFINE_DESTORY_VK_OBJECT(RenderPass)
    DEFINE_DESTORY_VK_OBJECT(Sampler)
    DEFINE_DESTORY_VK_OBJECT(ShaderModule)

#define DEFINE_CREATE_PIPELINES(T) \
    VkResult CreatePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const Vk##T##PipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) \
    { \
        return vkCreate##T##Pipelines(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines); \
    }

    DEFINE_CREATE_PIPELINES(Graphics)
    DEFINE_CREATE_PIPELINES(Compute)

#define DEFINE_GET_REQUIREMENTS(T) \
    void GetRequirements(Vk##T object, VkMemoryRequirements *pMemoryRequirements) \
    { \
        vkGet##T##MemoryRequirements(handle, object, pMemoryRequirements); \
    }

    DEFINE_GET_REQUIREMENTS(Buffer)
    DEFINE_GET_REQUIREMENTS(Image)

#define DEFINE_BIND_MEMORY(T) \
    VkResult BindMemory(Vk##T buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) \
    { \
        return vkBind##T##Memory(handle, buffer, memory, memoryOffset); \
    }

    DEFINE_BIND_MEMORY(Buffer)
    DEFINE_BIND_MEMORY(Image)

    VkResult AllocateMemory(const VkMemoryAllocateInfo *pAllocateInfo, VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
    {
        return vkAllocateMemory(handle, pAllocateInfo, pAllocator, pMemory);
    }

    void FreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator = nullptr)
    {
        vkFreeMemory(handle, memory, pAllocator);
    }

    VkResult AllocateDescriptorSet(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSets)
    {
        return descriptorPool->Allocate(pDescriptorSetLayout, pDescriptorSets);
    }

    void FreeDescriptorSet(VkDescriptorSet *pDescriptorSets, uint32_t size = 1)
    {
        if (pDescriptorSets != nullptr)
        {
            descriptorPool->Free(pDescriptorSets, size);
        }
    }

    void UpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies)
    {
        vkUpdateDescriptorSets(handle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }

    VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
    {
        return vkMapMemory(handle, memory, offset, size, flags, ppData);
    }

    void UnmapMemory(VkDeviceMemory memory)
    {
        vkUnmapMemory(handle, memory);
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

    operator VkDevice() const
    {
        return handle;
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<PhysicalDevice, T>())
        {
            return physicalDevice;
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return queues;
        }
        if constexpr (IsPrimitiveOf<CommandPool, T>())
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

    CommandBuffer *RequestCommandBuffer(Level level)
    {
        return commandPool->RequestBuffer(level);
    }

    VkFence RequestFence()
    {
        return fencePool->Request();
    }

    void Discard(CommandBuffer *commandBuffer)
    {
        commandPool->DiscardBuffer(commandBuffer);
    }

    void Discard(VkFence *pFence)
    {
        fencePool->Discard(pFence);
    }

    CommandBuffer *Begin()
    {
        auto *copyCmd = RequestCommandBuffer(Level::Primary);
        copyCmd->Begin();
        return copyCmd;
    }

    template <Queue::Type T>
    void End(CommandBuffer *copyCmd)
    {
        copyCmd->End();

        auto &queue = FindQueueByType(T, 0);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &copyCmd->Handle();

        auto fence = RequestFence();

        queue.Submit(submitInfo, fence);
        Check(Wait(&fence, 1, VK_TRUE, FencePool::Timeout));

        Discard(&fence);
        Discard(copyCmd);
    }

    template <class T>
    void Transfer(T &&process)
    {
        auto *copyCmd = Begin();

        process(copyCmd);

        End<Queue::Type::Transfer>(copyCmd);
    }

    template <class T>
    void Compute(T &&process)
    {
        auto *copyCmd = Begin();

        process(copyCmd);

        End<Queue::Type::Compute>(copyCmd);
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
