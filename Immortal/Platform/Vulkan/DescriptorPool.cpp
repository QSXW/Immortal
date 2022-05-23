#include "impch.h"

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
    for (auto &handle : handles)
    {
        device->DestroyAsync(handle);
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

}

void DescriptorPool::Reset()
{

}

VkDescriptorPool DescriptorPool::Create()
{
    VkDescriptorPool handle;

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes    = poolSize.data();
    createInfo.maxSets       = poolSize.size() * MaxSetsPerPool;

    Check(vkCreateDescriptorPool(*device, &createInfo, nullptr, &handle));

    return handle;
}

VkResult DescriptorPool::AllocateInternal(const VkDescriptorSetLayout * pDescriptorSetLayout, VkDescriptorSet * pDescriptorSet, uint32_t count)
{
    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    createInfo.descriptorPool     = handles.back();
    createInfo.descriptorSetCount = count;
    createInfo.pSetLayouts        = pDescriptorSetLayout;

    return vkAllocateDescriptorSets(*device, &createInfo, pDescriptorSet);
}

}
}
