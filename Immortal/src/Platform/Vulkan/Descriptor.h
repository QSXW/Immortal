#pragma once

#include "Common.h"
#include "Buffer.h"

namespace Immortal
{
namespace Vulkan
{

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
        info.sampler = sampler;
        info.imageView = imageView;
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

    void Update(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
    {
        info.buffer = buffer;
        info.offset = offset;
        info.range  = range;
    }

    operator const VkDescriptorBufferInfo*() const
    {
        return &info;
    }

    VkDescriptorBufferInfo info{};
};

struct UniformDescriptor
{
    UniformDescriptor(VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) :
        descriptor{ type }
    {

    }

    std::shared_ptr<Buffer> buffer;
    BufferDescriptor descriptor;
};

}
}
