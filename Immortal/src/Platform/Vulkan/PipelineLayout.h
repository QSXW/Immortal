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

    PipelineLayout(PipelineLayout &&other) :
        device{ other.device },
        handle{ other.handle }
    {
        other.device = VK_NULL_HANDLE;
        other.handle = VK_NULL_HANDLE;
    }

    PipelineLayout(const PipelineLayout &other) :
        device{ other.device },
        handle{ other.handle }
    {

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

    PipelineLayout &operator=(PipelineLayout &&other)
    {
        device = other.device;
        handle = other.handle;
        other.device = VK_NULL_HANDLE;
        other.handle = VK_NULL_HANDLE;

        return *this;
    }

    PipelineLayout &operator=(const PipelineLayout &other)
    {
        device = other.device;
        handle = other.handle;
        
        return *this;
    }

    VkDevice device{ VK_NULL_HANDLE };

    VkPipelineLayout handle{ VK_NULL_HANDLE };
};

}
}
