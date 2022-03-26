#include "impch.h"
#include "Device.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Common.h"
#include "PhysicalDevice.h"
#include "Framework/Async.h"

namespace Immortal
{
namespace Vulkan
{

static inline std::array<const char *, 5> VideoCodecExtensions = {
    "VK_KHR_synchronization2",
    "VK_KHR_video_queue",
    "VK_KHR_video_decode_queue",
    "VK_KHR_video_encode_queue",
    "VK_KHR_sampler_ycbcr_conversion"
};

Device::Device(PhysicalDevice *physicalDevice, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions) :
    physicalDevice{ physicalDevice },
    surface(surface)
{
    uint32_t propsCount = U32(physicalDevice->QueueFamilyProperties.size());
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(propsCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
    std::vector<std::vector<float>>      queueProps(propsCount);

    for (uint32_t index = 0; index < propsCount; index++)
    {
        const VkQueueFamilyProperties &prop = physicalDevice->QueueFamilyProperties[index];
        if (physicalDevice->HighPriorityGraphicsQueue && QueueFailyIndex(VK_QUEUE_GRAPHICS_BIT) == index)
        {
            queueProps[index].reserve(prop.queueCount);
            queueProps[index].push_back(1.0f);
            for (uint32_t i = 1; i < prop.queueCount; i++)
            {
                queueProps[index].push_back(0.5f);
            }
        }
        else
        {
            queueProps[index].resize(prop.queueCount, 0.5f);
        }
        queueCreateInfos[index].pNext            = nullptr;
        queueCreateInfos[index].queueFamilyIndex = index;
        queueCreateInfos[index].queueCount       = prop.queueCount;
        queueCreateInfos[index].pQueuePriorities = queueProps[index].data();
    }

    std::vector<VkExtensionProperties> deviceExtensions;
    uint32_t deviceExtensionCount;
    Check(physicalDevice->EnumerateDeviceExtensionProperties(nullptr, &deviceExtensionCount, nullptr));

    deviceExtensions.resize(deviceExtensionCount);
    Check(physicalDevice->EnumerateDeviceExtensionProperties(nullptr, &deviceExtensionCount, deviceExtensions.data()));

    for (auto &e : deviceExtensions)
    {
        availableExtensions.insert(e.extensionName);
    }

#if SLDEBUG
    if (!deviceExtensions.empty())
    {
        LOG::DEBUG<isLogNeed>("Device supports the following extensions: ");
        for (auto &ext : deviceExtensions)
        {
            LOG::DEBUG<isLogNeed>("  \t{0}", ext.extensionName);
        }
    }
#endif
    bool hasGetMemoryRequirements2 = IsExtensionSupport("VK_KHR_get_memory_requirements2");
    bool hasDedicatedAllocation    = IsExtensionSupport("VK_KHR_dedicated_allocation");
    if (hasGetMemoryRequirements2 && hasDedicatedAllocation)
    {
        enabledExtensions.emplace_back("VK_KHR_get_memory_requirements2");
        enabledExtensions.emplace_back("VK_KHR_dedicated_allocation");

        LOG::DEBUG<isLogNeed>("Dedicated Allocation enabled");
    }

    if (IsExtensionSupport("VK_KHR_performance_query") && IsExtensionSupport("VK_EXT_host_query_reset"))
    {
        auto &perfCounterFeatures       = physicalDevice->RequestExtensionFeatures<VkPhysicalDevicePerformanceQueryFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR);
        auto &host_query_reset_features = physicalDevice->RequestExtensionFeatures<VkPhysicalDeviceHostQueryResetFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES);
        LOG::DEBUG<isLogNeed>("Performance query enabled");
    }

    std::vector<const char*> unsupportedExtensions{};
    for (auto &ext : requestedExtensions)
    {
        if (IsExtensionSupport(ext.first))
        {
            enabledExtensions.emplace_back(ext.first);
        }
        else
        {
            unsupportedExtensions.emplace_back(ext.first);
        }
    }

    if (!enabledExtensions.empty())
    {
        LOG::DEBUG<isLogNeed>("Device supports the following requested extensions:");
        for (auto& ext : enabledExtensions)
        {
            LOG::DEBUG<isLogNeed>("  \t{0}", ext);
        }
    }

    if (!unsupportedExtensions.empty())
    {
        for (auto& ext : unsupportedExtensions)
        {
            auto isOptional = requestedExtensions[ext];
            if (isOptional)
            {
                LOG::WARN("Optional device extension {0} not available. Some features may be disabled", ext);
            }
            else
            {
                LOG::ERR("Required device extension {0} not available. Stop running!", ext);
                Check(VK_ERROR_EXTENSION_NOT_PRESENT);
            }
        }
    }

    /* Try to enable the video queue extension */
    for (const auto &extension : VideoCodecExtensions)
    {
        if (IsExtensionSupport(extension))
        {
            enabledExtensions.emplace_back(extension);
        }
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = physicalDevice->LastRequestedExtensionFeature;
    createInfo.queueCreateInfoCount    = U32(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.enabledExtensionCount   = U32(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.pEnabledFeatures        = &physicalDevice->RequestedFeatures;

    Check(physicalDevice->CreateDevice(&createInfo, &handle));

    queues.resize(propsCount);
    for (uint32_t queueFamilyIndex = 0U; queueFamilyIndex < propsCount; queueFamilyIndex++)
    {
        const auto& queueFamilyProps = physicalDevice->QueueFamilyProperties[queueFamilyIndex];
        VkBool32 presentSupported = surface ? physicalDevice->IsPresentSupported(surface, queueFamilyIndex) : false;

        for (uint32_t queueIndex = 0U; queueIndex < queueFamilyProps.queueCount; queueIndex++)
        {
            queues[queueFamilyIndex].emplace_back(this, queueFamilyIndex, queueFamilyProps, presentSupported, queueIndex);
        }
    }

    VmaVulkanFunctions vmaVulkanFunc{};
    vmaVulkanFunc.vkAllocateMemory                    = vkAllocateMemory;
    vmaVulkanFunc.vkBindBufferMemory                  = vkBindBufferMemory;
    vmaVulkanFunc.vkBindImageMemory                   = vkBindImageMemory;
    vmaVulkanFunc.vkCreateBuffer                      = vkCreateBuffer;
    vmaVulkanFunc.vkCreateImage                       = vkCreateImage;
    vmaVulkanFunc.vkDestroyBuffer                     = vkDestroyBuffer;
    vmaVulkanFunc.vkDestroyImage                      = vkDestroyImage;
    vmaVulkanFunc.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vmaVulkanFunc.vkFreeMemory                        = vkFreeMemory;
    vmaVulkanFunc.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vmaVulkanFunc.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vmaVulkanFunc.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vmaVulkanFunc.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vmaVulkanFunc.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vmaVulkanFunc.vkMapMemory                         = vkMapMemory;
    vmaVulkanFunc.vkUnmapMemory                       = vkUnmapMemory;
    vmaVulkanFunc.vkCmdCopyBuffer                     = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = *physicalDevice;
    allocatorInfo.device         = handle;
    allocatorInfo.instance       = physicalDevice->Get<Instance&>();
    allocatorInfo.pVulkanFunctions = &vmaVulkanFunc;

    if (IsExtensionSupport(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && IsEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    if (hasGetMemoryRequirements2 && hasDedicatedAllocation)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        vmaVulkanFunc.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vmaVulkanFunc.vkGetImageMemoryRequirements2KHR  = vkGetImageMemoryRequirements2KHR;
    }

    Check(vmaCreateAllocator(&allocatorInfo, &memoryAllocator));

    commandPool.reset(new CommandPool{ this, FindQueueByType(Queue::Type::Graphics | Queue::Type::Compute, 0).Get<Queue::FamilyIndex>() });
    compute.commandPool.reset(new CommandPool{ this, FindQueueByType(Queue::Type::Compute, 0).Get<Queue::FamilyIndex>() });
    fencePool.reset(new FencePool{ this });

    descriptorPool.reset(new DescriptorPool{ this, Limit::PoolSize });

    for (size_t i = 0; i < SL_ARRAY_LENGTH(compute.commandBuffers); i++)
    {
        compute.commandBuffers[i] = compute.commandPool->RequestBuffer(Level::Primary);
    }

    EnableGlobal();
}

uint32_t Device::QueueFailyIndex(VkQueueFlagBits requestFlags)
{
    const auto &queueFamilyProperties = physicalDevice->QueueFamilyProperties;

    VkQueueFlags mask = 0;

    if (requestFlags & VK_QUEUE_COMPUTE_BIT)
    {
        mask |= VK_QUEUE_GRAPHICS_BIT;
    }

    if (requestFlags & VK_QUEUE_TRANSFER_BIT)
    {
        mask |= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    }

    for (uint32_t i = 0; i < U32(queueFamilyProperties.size()); i++)
    {
        auto queueFlags = queueFamilyProperties[i].queueFlags;
        if (queueFlags & requestFlags && !(queueFlags & mask))
        {
            return i;
            break;
        }
    }

    LOG::ERR("Counld not find a matching queue family index");
    return 0;
}

Device::~Device()
{
    Wait();
    commandPool.reset();
    compute.commandPool.reset();
    fencePool.reset();
    descriptorPool.reset();

    for (auto &queue : destroyCoroutine.queues)
    {
        while (!queue.empty())
        {
            std::function<void()> func = queue.front();
            queue.pop();
            func();
        }
    }

    if (memoryAllocator != VK_NULL_HANDLE)
    {
        VmaStats stats;
        vmaCalculateStats(memoryAllocator, &stats);
        LOG::INFO("Total device memory leaked: {} Bytes({} MB).",
            stats.total.usedBytes,
            stats.total.usedBytes / (1024.0f * 1024.0f)
        );
        vmaDestroyAllocator(memoryAllocator);
    }

    IfNotNullThen(vkDestroyDevice, handle);
}

Queue &Device::SuitableGraphicsQueue()
{
    for (uint32_t familyIndex = 0; familyIndex < queues.size(); familyIndex++)
    {
        Queue &firstQueue = queues[familyIndex][0];

        uint32_t queueCount = firstQueue.Properties().queueCount;

        if (firstQueue.IsPresentSupported() && 0 < queueCount)
        {
            return queues[familyIndex][0];
        }
    }

    return FindQueueByType(Queue::Type::Graphics, 0);
}

Queue &Device::FindQueueByType(Queue::Type type, uint32_t queueIndex)
{
    VkQueueFlags flags = VkQueueFlags(type);

    for (uint32_t familyIndex = 0U; familyIndex < queues.size(); familyIndex++)
    {
        Queue& firstQueue = queues[familyIndex][0];

        VkQueueFlags queueFlags = firstQueue.Properties().queueFlags;
        uint32_t     queueCount = firstQueue.Properties().queueCount;

        if (((queueFlags & flags) == flags) && queueIndex < queueCount)
        {
            return queues[familyIndex][queueIndex];
        }
    }

    LOG::ERR("Queue not found");
    return queues[0][0];
}

uint32_t Device::GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memoryTypeFound)
{
    for (uint32_t i = 0; i < physicalDevice->MemoryProperties.memoryTypeCount; i++)
    {
        if (bits & 1)
        {
            if ((physicalDevice->MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memoryTypeFound)
                {
                    *memoryTypeFound = true;
                }
                return i;
            }
        }
        bits >>= 1;
    }

    if (memoryTypeFound)
    {
        *memoryTypeFound = false;
        return 0;
    }
    else
    {
        SLASSERT(nullptr && "Could not find a matching memory type");
        return -1;
    }
}

void Device::DestroyObjects()
{
    auto &queue = destroyCoroutine.queues[destroyCoroutine.freeing];
    auto &mutex = destroyCoroutine.mutex;

    SLROTATE(destroyCoroutine.working, destroyCoroutine.queues.size());
    SLROTATE(destroyCoroutine.freeing, destroyCoroutine.queues.size());

    /* all objects in queue is safe to destory now */
    Async::Execute([&] {
        while (!queue.empty())
        {
            std::function<void()> func;
            {
                std::unique_lock<std::mutex> lock{ mutex };
                func = queue.front();
                queue.pop();
            }
            func();
        }
        });
}

VkResult Device::AllocateDescriptorSet(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSets)
{
    return descriptorPool->Allocate(pDescriptorSetLayout, pDescriptorSets);
}

void Device::FreeDescriptorSet(VkDescriptorSet *pDescriptorSets, uint32_t size)
{
    if (pDescriptorSets != nullptr)
    {
        descriptorPool->Free(pDescriptorSets, size);
    }
}

}
}
