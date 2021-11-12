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

    PipelineLayout(VkDevice device, uint32_t layoutCount, VkDescriptorSetLayout *pLayout) :
        device{ device }
    {
        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = layoutCount;
        createInfo.pSetLayouts    = pLayout;

        vkCreatePipelineLayout(device, &createInfo, nullptr, &handle);
    }

    ~PipelineLayout()
    {
        IfNotNullThen(vkDestroyPipelineLayout, device, handle, nullptr);
    }

    operator VkPipelineLayout&()
    {
        return handle;
    }

    operator VkPipelineLayout() const
    {
        return handle;
    }

    VkDevice device{ VK_NULL_HANDLE };

    VkPipelineLayout handle{ VK_NULL_HANDLE };
};

}
}
