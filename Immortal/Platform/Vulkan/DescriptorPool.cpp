#include "impch.h"

#include "DescriptorPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

DescriptorPool::DescriptorPool(Device *device, const DescriptorSetLayout &layout, UINT32 poolSize) :
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
    allocatedCount += count;
    if (allocatedCount > MaxSetsPerPool)
    {
        handles.push_back(Create());
        allocatedCount = 0;
    }
    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    createInfo.descriptorPool     = handles.back();
    createInfo.descriptorSetCount = count;
    createInfo.pSetLayouts        = pDescriptorSetLayout;

    return vkAllocateDescriptorSets(*device, &createInfo, pDescriptorSet);
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
    createInfo.maxSets       = poolSize.size() * 1000;

    Check(vkCreateDescriptorPool(*device, &createInfo, nullptr, &handle));

    return handle;
}

}
}
