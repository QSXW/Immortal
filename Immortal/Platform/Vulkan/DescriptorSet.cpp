#include "DescriptorSet.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

DescriptorSet::DescriptorSet() :
    device{},
    handle{}
{

}

DescriptorSet::DescriptorSet(Device *device, const VkDescriptorSetLayout &descriptorSetLayout) :
    device{ device }
{
    Check(device->AllocateDescriptorSet(&descriptorSetLayout, &handle));
}

DescriptorSet::DescriptorSet(const VkDescriptorSet other)
{
    handle = other;
}

DescriptorSet::DescriptorSet(DescriptorSet &&other) :
    DescriptorSet{}
{
    other.Swap(*this);
}

DescriptorSet &DescriptorSet::operator =(DescriptorSet &&other)
{
    DescriptorSet(std::move(other)).Swap(*this);
    return *this;
}

void DescriptorSet::Swap(DescriptorSet &other)
{
    std::swap(device, other.device);
    std::swap(handle, other.handle);
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
