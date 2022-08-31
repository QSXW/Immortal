#pragma once

#include <string>
#include <map>

#include "Common.h"
#include "Render/Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

struct ImageDescriptor : public Descriptor, public VkDescriptorImageInfo
{
    using Primitive = VkDescriptorImageInfo;

    ImageDescriptor()
    {

    }

    void Update(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
    {
        Primitive::sampler     = sampler;
        Primitive::imageView   = imageView;
        Primitive::imageLayout = imageLayout;
    }
};

struct BufferDescriptor : public Descriptor, public VkDescriptorBufferInfo
{
    using Primitive = VkDescriptorBufferInfo;

    BufferDescriptor() 
    {
        buffer = VK_NULL_HANDLE;
        offset = 0;
        range  = VK_WHOLE_SIZE;
    }

    void Update(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
    {
        Primitive::buffer = buffer;
        Primitive::offset = offset;
        Primitive::range  = range;
    }
};

using WriteDescriptorSetMap = std::map<std::string, size_t>;

struct DescriptorSetUpdater
{
    DescriptorSetUpdater() = default;

    void Emplace(const std::string &key, const VkWriteDescriptorSet &&writeDescriptorSet)
    {
        auto binding = writeDescriptorSet.dstBinding;
        if (binding >= WriteDescriptorSets.size())
        {
            WriteDescriptorSets.resize(binding + 1);
        }
        WriteDescriptorSets[binding] = writeDescriptorSet;
        Map[key] = binding;
    }

    VkWriteDescriptorSet &operator [](size_t index)
    {
        return WriteDescriptorSets[index];
    }

    void Update(VkDevice device)
    {
        vkUpdateDescriptorSets(device, WriteDescriptorSets.size(), WriteDescriptorSets.data(), 0, nullptr);
    }

    void Set(VkDescriptorSet descriptorSet)
    {
        for (auto &w : WriteDescriptorSets)
        {
            w.dstSet = descriptorSet;
        }
    }

    void Clear()
    {
        for (auto &w : WriteDescriptorSets)
        {
            w.pBufferInfo = nullptr;
            w.pImageInfo  = nullptr;
        }
    }

    bool Ready()
    {
        for (auto &writeDescriptor : WriteDescriptorSets)
        {
			if (writeDescriptor.sType == VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET && !writeDescriptor.pImageInfo && !writeDescriptor.pBufferInfo)
            {
                LOG::WARN<false>("There is a(n) \"{0}\" binding on slot \"{1}\" but no input yet",
                    writeDescriptor.descriptorType <= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ? 
                    "Image(Sampler)" : "(Uniform) Buffer",
                    writeDescriptor.dstBinding);
                return false;
            }
        }

        LOG::DEBUG<false>("All descriptor set binded");
        return true;
    }

    bool Set(const std::string &name, const BufferDescriptor *descriptor)
    {
        auto it = Map.find(name);
        if (it == Map.end())
        {
            LOG::WARN("There is no any buffer named \"{0}\". Buffer updating is invalid.", name);
            return false;
        }
        auto index = it->second;

        WriteDescriptorSets[index].pBufferInfo = descriptor;
        return true;
    }

    WriteDescriptorSetMap Map;

    std::vector<VkWriteDescriptorSet> WriteDescriptorSets;
};

}
}
