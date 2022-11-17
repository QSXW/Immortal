#pragma once

#include "Common.h"
#include "Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class DescriptorSet
{
public:
    using Primitive = VkDescriptorSet;
    VKCPP_OPERATOR_HANDLE()

public:
    DescriptorSet();

    DescriptorSet(Device *device, const VkDescriptorSetLayout &descriptorSetLayout);

    DescriptorSet(const VkDescriptorSet other);

    DescriptorSet(DescriptorSet &&other);

    DescriptorSet &operator =(DescriptorSet &&other);

    ~DescriptorSet();

    void Swap(DescriptorSet &other);

    void Update(VkWriteDescriptorSet *desc);

    template <class T>
    void Update(T descriptorInfo, VkDescriptorType type, uint32_t slot = 0)
    {
        VkWriteDescriptorSet desc{};
        desc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc.pNext           = nullptr;
        desc.dstBinding      = slot;
        desc.dstSet          = handle;
        desc.descriptorCount = 1;
        desc.descriptorType  = type;

        if constexpr (IsPrimitiveOf<ImageDescriptor, T>())
        {
            desc.pImageInfo = &descriptorInfo;
            Update(&desc);
            return;
        }
        if constexpr (IsPrimitiveOf<BufferDescriptor, T>())
        {
            desc.pBufferInfo = &descriptorInfo;
            Update(&desc);
            return;
        }
    }

    operator uint64_t() const
    {
        return (uint64_t)(handle);
    }

    DescriptorSet(const DescriptorSet &other) = delete;
    DescriptorSet &operator=(const DescriptorSet &other) = delete;

protected:
    Device *device{ nullptr };
};

}
}
