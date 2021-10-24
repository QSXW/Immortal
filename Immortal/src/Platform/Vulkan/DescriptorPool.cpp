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
    device{ device }
{
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes    = poolSize.data();
    createInfo.maxSets       = poolSize.size() * 1000;

    vkCreateDescriptorPool(device->Handle(), &createInfo, nullptr, &handle);
}

DescriptorPool::~DescriptorPool()
{
    IfNotNullThen(vkDestroyDescriptorPool, device->Handle(), handle, nullptr);
}

VkResult DescriptorPool::Allocate(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSet)
{
    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    createInfo.descriptorPool     = handle;
    createInfo.descriptorSetCount = 1;
    createInfo.pSetLayouts        = pDescriptorSetLayout;

    return vkAllocateDescriptorSets(device->Handle(), &createInfo, pDescriptorSet);
}

void DescriptorPool::Free(VkDescriptorSet *pDescriptorSet, uint32_t size)
{
    vkFreeDescriptorSets(device->Handle(), handle, size, pDescriptorSet);
}

}
}
