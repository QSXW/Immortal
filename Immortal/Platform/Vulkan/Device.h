#pragma once
#include "Core.h"

#include "Common.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "CommandPool.h"
#include "FencePool.h"
#include "Interface/IObject.h"

#include <queue>
#include <future>

namespace Immortal
{
namespace Vulkan
{

class DescriptorPool;
class Swapchain;
class Device : public IObject
{
public:
    static inline Device *That = nullptr;

    static inline constexpr bool isLogNeed = false;

    void EnableGlobal()
    {
        That = this;
    }

    using Primitive = VkDevice;
    VKCPP_OPERATOR_HANDLE()

public:
    Device() = default;

    Device(PhysicalDevice *physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});

    ~Device();

    VkResult AllocateDescriptorSet(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSets);

    void FreeDescriptorSet(VkDescriptorSet *pDescriptorSets, uint32_t size = 1);

    uint32_t QueueFailyIndex(VkQueueFlagBits queueFlag);

    uint32_t QueueFailyIndex(Queue::Type type)
    {
        return QueueFailyIndex(VkQueueFlagBits(type));
    }

    Queue &FindQueueByType(Queue::Type type, uint32_t queueIndex);

    Queue &SuitableGraphicsQueue();

    uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memoryTypeFound = nullptr);

    void DestroyObjects();

    template <class T>
    void DestroyAsync(T task)
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock{ destroyCoroutine.mutex };
            destroyCoroutine.queues[destroyCoroutine.working].push([=]() -> void {
                (*wrapper)();
                });
        }
    }

    Queue *SuitableGraphicsQueuePtr()
    {
        auto &queue = SuitableGraphicsQueue();
        return &queue;
    }

    VkFormat DepthFormat(bool depthOnly = false)
    {
        return physicalDevice->GetSuitableDepthFormat(depthOnly);
    }

    bool IsEnabled(const std::string &extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(), [extension](const std::string &enabledExtension)
        {
            return Equals(extension.c_str(), enabledExtension.c_str());
        }) != enabledExtensions.end();
    }

    bool IsExtensionSupport(const std::string &extension) const
    {
        return availableExtensions.find(extension) != availableExtensions.end();
    }

public:
#define DEFINE_CREATE_VK_OBJECT(T) \
    VkResult Create(const Vk##T##CreateInfo *pCreateInfo, Vk##T *pObject, VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T(handle, pCreateInfo, pAllocator, pObject); \
    }

#define DEFINE_CREATE_VK_KHR_OBJECT(T) \
    VkResult Create(const Vk##T##CreateInfoKHR *pCreateInfo, Vk##T##KHR *pObject, VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T##KHR(handle, pCreateInfo, pAllocator, pObject); \
    }

#define DEFINE_DESTORY_VK_OBJECT(T) \
    void Destroy(Vk##T object, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        if (object != VK_NULL_HANDLE) \
        { \
            vkDeviceWaitIdle(handle); \
            vkDestroy##T(handle, object, pAllocator); \
        } \
    } \
    void DestroyAsync(Vk##T object, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        if (object != VK_NULL_HANDLE) \
        { \
            std::unique_lock<std::mutex> lock{ destroyCoroutine.mutex }; \
            destroyCoroutine.queues[destroyCoroutine.working].push([=]{ \
                vkDestroy##T(handle, object, pAllocator); \
            }); \
        } \
    }

#define DEFINE_RESET_OBJECT(T) \
    void Reset(Vk##T object, Vk##T##ResetFlags flags) \
    { \
        Check(vkReset##T(handle, object, flags)); \
    }

#define DEFINE_CREATE_PIPELINES(T) \
    VkResult CreatePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const Vk##T##PipelineCreateInfo *pCreateInfos, VkPipeline *pPipelines, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T##Pipelines(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines); \
    }

#define DEFINE_GET_REQUIREMENTS(T) \
    void GetRequirements(Vk##T object, VkMemoryRequirements *pMemoryRequirements) \
    { \
        vkGet##T##MemoryRequirements(handle, object, pMemoryRequirements); \
    }

#define DEFINE_BIND_MEMORY(T) \
    void BindMemory(Vk##T buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) \
    { \
        Check(vkBind##T##Memory(handle, buffer, memory, memoryOffset)); \
    } \
    void BindMemory(const VkBind##T##MemoryInfo *bindInfos, uint32_t count = 1) \
    { \
        Check(vkBind##T##Memory2(handle, count, bindInfos)); \
    }

    DEFINE_CREATE_VK_OBJECT(Buffer)
    DEFINE_CREATE_VK_OBJECT(CommandPool)
    DEFINE_CREATE_VK_OBJECT(DescriptorPool)
    DEFINE_CREATE_VK_OBJECT(DescriptorSetLayout)
    DEFINE_CREATE_VK_OBJECT(Fence)
    DEFINE_CREATE_VK_OBJECT(Framebuffer)
    DEFINE_CREATE_VK_OBJECT(Image)
    DEFINE_CREATE_VK_OBJECT(ImageView)
    DEFINE_CREATE_VK_OBJECT(PipelineLayout)
    DEFINE_CREATE_VK_OBJECT(RenderPass)
    DEFINE_CREATE_VK_OBJECT(Sampler)
    DEFINE_CREATE_VK_OBJECT(Semaphore)
    DEFINE_CREATE_VK_OBJECT(ShaderModule)
    DEFINE_CREATE_VK_OBJECT(PipelineCache)

    DEFINE_CREATE_VK_KHR_OBJECT(Swapchain)
    DEFINE_CREATE_VK_KHR_OBJECT(VideoSession)

    DEFINE_DESTORY_VK_OBJECT(Buffer)
    DEFINE_DESTORY_VK_OBJECT(CommandPool)
    DEFINE_DESTORY_VK_OBJECT(DescriptorPool)
    DEFINE_DESTORY_VK_OBJECT(DescriptorSetLayout)
    DEFINE_DESTORY_VK_OBJECT(Fence)
    DEFINE_DESTORY_VK_OBJECT(Framebuffer)
    DEFINE_DESTORY_VK_OBJECT(Image)
    DEFINE_DESTORY_VK_OBJECT(ImageView)
    DEFINE_DESTORY_VK_OBJECT(Pipeline)
    DEFINE_DESTORY_VK_OBJECT(PipelineLayout)
    DEFINE_DESTORY_VK_OBJECT(RenderPass)
    DEFINE_DESTORY_VK_OBJECT(Sampler)
    DEFINE_DESTORY_VK_OBJECT(Semaphore)
    DEFINE_DESTORY_VK_OBJECT(ShaderModule)
    DEFINE_DESTORY_VK_OBJECT(SwapchainKHR)
    DEFINE_DESTORY_VK_OBJECT(PipelineCache)

    DEFINE_RESET_OBJECT(CommandPool)
    DEFINE_RESET_OBJECT(DescriptorPool)

    DEFINE_CREATE_PIPELINES(Graphics)
    DEFINE_CREATE_PIPELINES(Compute)

    DEFINE_GET_REQUIREMENTS(Buffer)
    DEFINE_GET_REQUIREMENTS(Image)

    DEFINE_BIND_MEMORY(Buffer)
    DEFINE_BIND_MEMORY(Image)

#define DEFINE_VMA_CREATE_DESTROY_OBJECT(T) \
    VkResult Create##T(const Vk##T##CreateInfo *pCreateInfo, const VmaAllocationCreateInfo *pAllocationCreateInfo, Vk##T *pObject,VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo = nullptr) \
    { \
        return vmaCreate##T(memoryAllocator, pCreateInfo, pAllocationCreateInfo, pObject, pAllocation, pAllocationInfo); \
    } \
    VkResult Create(const Vk##T##CreateInfo *pCreateInfo, const VmaAllocationCreateInfo *pAllocationCreateInfo, Vk##T *pObject, VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo = nullptr) \
    { \
        return vmaCreate##T(memoryAllocator, pCreateInfo, pAllocationCreateInfo, pObject, pAllocation, pAllocationInfo); \
    } \
    void Destroy##T(Vk##T object, VmaAllocation allocation) \
    { \
        vmaDestroy##T(memoryAllocator, object, allocation); \
    } \
    void Destroy(Vk##T object, VmaAllocation allocation) \
    { \
        vmaDestroy##T(memoryAllocator, object, allocation); \
    }

    DEFINE_VMA_CREATE_DESTROY_OBJECT(Buffer)
    DEFINE_VMA_CREATE_DESTROY_OBJECT(Image)

    VkResult AllocateMemory(const VkMemoryAllocateInfo *pAllocateInfo, VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
    {
        return vkAllocateMemory(handle, pAllocateInfo, pAllocator, pMemory);
    }

    void FreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator = nullptr)
    {
        destroyCoroutine.queues[destroyCoroutine.working].push([=] {
            vkFreeMemory(handle, memory, pAllocator);
            }); 
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

    void GetQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue)
    {
        vkGetDeviceQueue(handle, queueFamilyIndex, queueIndex, pQueue);
    }

public:
    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<PhysicalDevice, T>())
        {
            return *physicalDevice;
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

    VkResult Wait(const VkFence *pFences, uint32_t fenceCount = 1, VkBool32 waitAll = VK_TRUE, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
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

    VkSurfaceKHR GetSurface() const
    {
        return surface;
    }

    VmaAllocator MemoryAllocator() const
    {
        return memoryAllocator;
    }

    VkResult GetSurfaceCapabilities(VkSurfaceCapabilitiesKHR *properties)
    {
        return physicalDevice->GetCapabilities(surface, properties);
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
    void End(CommandBuffer *cmdbuf)
    {
        cmdbuf->End();

        auto &queue = FindQueueByType(T, QueueFailyIndex(T));

        VkCommandBuffer commandBuffers[1] = { *cmdbuf };
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = commandBuffers;

        auto fence = RequestFence();

        queue.Submit(submitInfo, fence);
        Check(Wait(&fence, 1, VK_TRUE, FencePool::Timeout));

        Discard(&fence);
    }

    template <class T>
    void Transfer(T &&process)
    {
        auto *copyCmd = Begin();

        process(copyCmd);

        End<Queue::Type::Transfer>(copyCmd);
        Discard(copyCmd);
    }

    template <class T>
    void Compute(T &&process)
    {
        auto *cmd = Begin();

        process(cmd);

        End<Queue::Type::Compute>(cmd);
        Discard(cmd);
    }

    template <class T>
    void SubmitSync(T &&process)
    {
        auto *cmd = Begin();

        process(cmd);

        End<Queue::Type::Graphics>(cmd);
        Discard(cmd);
    }

    template <class T>
    void TransferAsync(T &&process)
    {
        CommandBuffer *cmdbuf = transfer->GetCurrentCommandBuffer();
        if (!cmdbuf->Recoding())
        {
            Transfer(process);
        }
        else
        {
            process(cmdbuf);
        }
    }

    template <class T>
    void ComputeAsync(T &&process)
    {
        CommandBuffer *cmdbuf = compute->GetCurrentCommandBuffer();
        if (!cmdbuf->Recoding())
        {
            Compute(process);
        }
        else
        {
            process(cmdbuf);
        }
    }

    template <class T>
    void Submit(T &&process)
    {
        CommandBuffer *cmdbuf = graphics->GetCurrentCommandBuffer();
        process(cmdbuf);
    }

    void BeginComputeThread()
    {
        graphics->Begin();
        compute->Begin();
        transfer->Begin();
    }

    void ExecuteComputeThread()
    {
        graphics->Sync();
        compute->Sync();
        transfer->Sync();
    }

private:
    PhysicalDevice *physicalDevice{ nullptr };

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    std::unordered_set<std::string> availableExtensions;

    std::vector<const char *> enabledExtensions;

    VmaAllocator memoryAllocator{ VK_NULL_HANDLE };

    std::vector<std::vector<Queue>> queues;

    std::unique_ptr<CommandPool> commandPool;

    std::unique_ptr<FencePool> fencePool;

    std::unique_ptr<DescriptorPool> descriptorPool;

    MonoRef<AsynchronousCommandBuffer> compute;

    MonoRef<AsynchronousCommandBuffer> transfer;

    MonoRef<AsynchronousCommandBuffer> graphics;

    struct {
        std::array<std::queue<std::function<void()>>, 6> queues;
        std::mutex mutex;
        uint32_t working = 0;
        uint32_t freeing = 1;
    } destroyCoroutine;
};
}
}
