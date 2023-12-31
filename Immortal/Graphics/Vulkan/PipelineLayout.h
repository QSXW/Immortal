#pragma once

#include "Common.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class PipelineLayout : public Handle<VkPipelineLayout>
{
public:
    VKCPP_SWAPPABLE(PipelineLayout)
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

    PipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo)
    {
		Check(vkCreatePipelineLayout(device, pCreateInfo, nullptr, &handle));
    }

    ~PipelineLayout()
    {

    }
    
    void Swap(PipelineLayout &other)
    {
        Handle::Swap(other);
    }
};

}
}
