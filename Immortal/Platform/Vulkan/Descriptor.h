#pragma once

#include <string>
#include <map>

#include "Common.h"
#include "Render/Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

struct ImageDescriptor : public Descriptor
{
    ImageDescriptor()
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

struct BufferDescriptor : public Descriptor, public VkDescriptorBufferInfo
{
    BufferDescriptor() 
    {
        buffer = VK_NULL_HANDLE;
        offset = 0;
        range  = VK_WHOLE_SIZE;
    }

    void Update(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
    {
        buffer = buffer;
        offset = offset;
        range  = range;
    }

    operator const VkDescriptorBufferInfo*() const
    {
        return this;
    }
};

using WriteDescriptorSetMap = std::map<std::string, size_t>;

struct DescriptorSetUpdater
{
    DescriptorSetUpdater() = default;

    void Emplace(const std::string &key, VkWriteDescriptorSet &&writeDescriptorSet)
    {
        WriteDescriptorSets.emplace_back(writeDescriptorSet);
        Map[key] = WriteDescriptorSets.size() - 1;
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
            w.pImageInfo = nullptr;
        }
    }

    bool Ready()
    {
        for (auto &writeDescriptor : WriteDescriptorSets)
        {
            if (!writeDescriptor.pImageInfo && !writeDescriptor.pBufferInfo)
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
