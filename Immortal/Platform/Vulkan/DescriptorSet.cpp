#include "DescriptorSet.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

DescriptorSet::DescriptorSet(Device *device, const VkDescriptorSetLayout &descriptorSetLayout) :
    device{ device }
{
    Check(device->AllocateDescriptorSet(&descriptorSetLayout, &handle));
}

DescriptorSet::DescriptorSet(const VkDescriptorSet other)
{
    handle = other;
}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Update(VkWriteDescriptorSet *desc)
{
    device->UpdateDescriptorSets(1, desc, 0, nullptr);
}

}
}
