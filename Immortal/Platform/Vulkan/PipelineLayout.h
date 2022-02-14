#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
struct PipelineLayout
{
    PipelineLayout() = default;

    PipelineLayout(VkDevice device, uint32_t layoutCount, VkDescriptorSetLayout *pLayout, const VkPushConstantRange *pRange, uint32_t rangeCount = 1)
    {
        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount         = layoutCount;
        createInfo.pSetLayouts            = pLayout;
        createInfo.pushConstantRangeCount = rangeCount;
        createInfo.pPushConstantRanges    = pRange;

        vkCreatePipelineLayout(device, &createInfo, nullptr, &handle);
    }

    PipelineLayout(PipelineLayout &&other) :
        handle{ other.handle }
    {
        other.handle = VK_NULL_HANDLE;
    }

    PipelineLayout(const PipelineLayout &other) :
        handle{ other.handle }
    {

    }

    ~PipelineLayout()
    {
        
    }

    VkPipelineLayout Handle() const
    {
        return handle;
    }

    operator VkPipelineLayout() const
    {
        return handle;
    }

    PipelineLayout &operator=(PipelineLayout &&other)
    {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;

        return *this;
    }

    PipelineLayout &operator=(const PipelineLayout &other)
    {
        handle = other.handle;
        
        return *this;
    }

private:
    VkPipelineLayout handle{ VK_NULL_HANDLE };
};

}
}
