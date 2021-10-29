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
        State state;
        Attachment attament;
    };

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> &shader);

    virtual ~Pipeline();

    virtual void Set(std::shared_ptr<Buffer> &buffer, Buffer::Type type = Buffer::Type::Vertex) override;

private:
    VkPrimitiveTopology ConvertType(PrimitiveType &type)
    {
        if (type == PrimitiveType::Line)
        {
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        if (type == PrimitiveType::Triangles)
        {
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
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
        
    }

private:
    Device *device{ nullptr };

    std::unique_ptr<Configuration> configuration;
};

}
}
