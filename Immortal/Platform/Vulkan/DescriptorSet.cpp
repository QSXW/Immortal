#pragma once

#include "Common.h"
#include "Device.h"
#include "DescriptorSet.h"

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
