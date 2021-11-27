#pragma once

#include "Common.h"
#include "Device.h"
#include "Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

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
        desc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc.pNext           = nullptr;
        desc.dstBinding      = slot;
        desc.dstSet          = handle;
        desc.descriptorCount = 1;
        desc.descriptorType  = descriptorInfo.Type();

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
        device->UpdateDescriptorSets(1, &desc, 0, nullptr);
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
