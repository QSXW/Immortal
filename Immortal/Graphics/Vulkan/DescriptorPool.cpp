#include "DescriptorPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

DescriptorPool::DescriptorPool(Device *device, const DescriptorSetLayout &layout, uint32_t poolSize) :
    device{ device }
{

}

DescriptorPool::DescriptorPool(Device *device, const std::vector<VkDescriptorPoolSize> &poolSize) :
    device{ device },
    poolSize{ poolSize }
{
    handles.push_back(Create());
}

DescriptorPool::~DescriptorPool()
{
    Destroy();
}

void DescriptorPool::Destroy()
{
    for (auto &handle : handles)
    {
        device->DestroyAsync(handle);
    }
    handles.clear();

    while (!cache.empty())
    {
        cache.pop();
    }
}

VkResult DescriptorPool::Allocate(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSet, uint32_t count)
{
    VkResult result = VK_SUCCESS;

    uint32_t free = MaxSetsPerPool - allocatedCount;
    if (count <= free)
    {
        allocatedCount += count;
        return AllocateInternal(pDescriptorSetLayout, pDescriptorSet, count);
    }

    if (free > 0)
    {
        result = AllocateInternal(pDescriptorSetLayout, pDescriptorSet, free);
    }
    if (result != VK_SUCCESS)
    {
        return result;
    }
    handles.push_back(Create());
    allocatedCount = count - free;

    return AllocateInternal(pDescriptorSetLayout, pDescriptorSet, allocatedCount);
}

void DescriptorPool::Free(VkDescriptorSet *pDescriptorSet, uint32_t size)
{
    if (!handles.empty())
    {
        for (size_t i = 0; i < size; i++)
        {
            cache.push(pDescriptorSet[i]);
        }
    }
}

void DescriptorPool::Reset()
{

}

VkDescriptorPool DescriptorPool::Create()
{
    VkDescriptorPool handle;

    VkDescriptorPoolCreateInfo createInfo = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
	    .maxSets       = uint32_t(poolSize.size() * MaxSetsPerPool),
	    .poolSizeCount = uint32_t(poolSize.size()),
        .pPoolSizes    = poolSize.data(),
    };

    Check(vkCreateDescriptorPool(*device, &createInfo, nullptr, &handle));

    return handle;
}

VkResult DescriptorPool::AllocateInternal(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSet, uint32_t count)
{
    if (count < cache.size())
    {
        for (uint32_t i = 0; i < count; i++)
        {
            pDescriptorSet[i] = cache.front();
            cache.pop();
        }

        return VK_SUCCESS;
    }

    VkDescriptorSetAllocateInfo createInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = handles.back(),
        .descriptorSetCount = count,
        .pSetLayouts        = pDescriptorSetLayout,
    };

    return vkAllocateDescriptorSets(*device, &createInfo, pDescriptorSet);
}

}
}
