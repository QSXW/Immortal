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
    DescriptorSet(Device *device, const VkDescriptorSetLayout &descriptorSetLayout);

    DescriptorSet(const VkDescriptorSet other);

    ~DescriptorSet();

    void Update(VkWriteDescriptorSet *desc);

    template <class T>
    void Update(T descriptorInfo, VkDescriptorType type, uint32_t slot = 0)
    {
        VkWriteDescriptorSet desc{};
        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc.pNext = nullptr;
        desc.dstBinding = slot;
        desc.dstSet = handle;
        desc.descriptorCount = 1;
        desc.descriptorType = type;

        if constexpr (IsPrimitiveOf<ImageDescriptor, T>())
        {
            desc.pImageInfo = descriptorInfo;
        }
        else if constexpr (IsPrimitiveOf<BufferDescriptor, T>())
        {
            desc.pBufferInfo = descriptorInfo;
        }
        else
        {
            static_assert(false && "Incorrect Descriptor Type");
        }
        Update(&desc);
    }

    operator uint64_t() const
    {
        return rcast<uint64_t>(handle);
    }

    operator const VkDescriptorSet&() const
    {
        return handle;
    }

private:
    Device *device{ nullptr };

    VkDescriptorSet handle{ VK_NULL_HANDLE };
};

}
}
