#pragma once

#include "Common.h"
#include "Device.h"

#include "Render/Pipeline.h"
#include "Shader.h"

namespace Immortal
{
namespace Vulkan
{

class Pipeline : public SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct State
    {
        VkPipelineVertexInputStateCreateInfo   vertexInput;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineRasterizationStateCreateInfo rasterization;
        VkPipelineColorBlendStateCreateInfo    colorBlend;
        VkPipelineDepthStencilStateCreateInfo  depthStencil;
        VkPipelineViewportStateCreateInfo      viewport;
        VkPipelineMultisampleStateCreateInfo   multiSample;
        VkPipelineDynamicStateCreateInfo       dynamic;
    };

    struct Attachment
    {
        VkPipelineColorBlendAttachmentState colorBlend;
    };

    struct Configuration
    {
        std::array<VkDynamicState, 2> dynamic{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        std::vector<VkVertexInputBindingDescription> vertexInputBidings;

        State state;

        Attachment attament;
    };

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> &shader);

    virtual ~Pipeline();

    virtual void Set(std::shared_ptr<SuperBuffer> &buffer) override;
    
    virtual void Set(const InputElementDescription &description) override;

private:
    VkPrimitiveTopology ConvertType(PrimitiveType &type)
    {
        if (type == PrimitiveType::Line)
        {
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    void INITVertex()
    {
        auto state = &configuration->state;
	    state->inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    state->inputAssembly.topology               = ConvertType(desc.PrimitiveType);
	    state->inputAssembly.flags                  = 0;
	    state->inputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    void INITLayout()
    {
        auto state = &configuration->state;
        auto &inputAttributeDescriptions = configuration->inputAttributeDescriptions;
        auto &vertexInputBidings         = configuration->vertexInputBidings;
        
        state->vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        state->vertexInput.vertexBindingDescriptionCount   = U32(vertexInputBidings.size());
        state->vertexInput.pVertexBindingDescriptions      = vertexInputBidings.data();
        state->vertexInput.vertexAttributeDescriptionCount = U32(inputAttributeDescriptions.size());
    }

private:
    Device *device{ nullptr };

    std::unique_ptr<Configuration> configuration;
};

}
}
