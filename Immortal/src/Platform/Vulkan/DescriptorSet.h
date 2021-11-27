#pragma once

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class Device;

struct Descriptor
{
    VkDescriptorType Type() const
    {
        return type;
    }

    VkDescriptorType type;
};

struct ImageDescriptor : public Descriptor
{
    ImageDescriptor(VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) :
        Descriptor{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
    {

    }

    void Update(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
    {
        info.sampler     = sampler;
        info.imageView   = imageView;
        info.imageLayout = imageLayout;
    }

    operator const VkDescriptorImageInfo*() const
    {
        return &info;
    }

    VkDescriptorImageInfo info{};
};

struct BufferDescriptor : public Descriptor
{
    BufferDescriptor(VkDescriptorType type) :
        Descriptor{ type }
    {

    }

    void Update()
    {

    }

    operator const VkDescriptorBufferInfo*() const
    {
        return &info;
    }

    VkDescriptorBufferInfo info{};
};

class DescriptorSet
{
public:
    DescriptorSet::DescriptorSet(Device *device, const VkDescriptorSetLayout &descriptorSetLayout) :
        device{ device }
    {
        Check(device->AllocateDescriptorSet(&descriptorSetLayout, &handle));
    }

    DescriptorSet::~DescriptorSet()
    {
        
    }

    template <class T>
    void Update(T descriptorInfo, uint32_t slot = 0)
    {
        VkWriteDescriptorSet desc{};
        if constexpr (IsPrimitiveOf<ImageDescriptor, T>())
        {
            desc.pImageInfo = descriptorInfo;
        }
        else if constexpr (IsPrimitiveOf<BufferDescriptor, T>())
        {
            desc.pBufferInfo = &(VkDescriptorBufferInfo&)descriptorInfo;
        }
        else constexpr
        {
            static_assert(false && "Incorrect Descriptor Type");
        }
        desc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc.pNext           = nullptr;
        desc.dstBinding      = slot;
        desc.dstSet          = handle;
        desc.descriptorCount = 1;
        desc.descriptorType  = descriptorInfo.Type();

        device->UpdateDescriptorSets(1, &desc, 0, nullptr);
    }

    operator uint64_t() const
    {
        return rcast<uint64_t>(handle);
    }

private:
    Device *device{ nullptr };

    VkDescriptorSet handle{ VK_NULL_HANDLE };
};

}
}
