#pragma once

#include "Common.h"

#include "Render/Pipeline.h"
#include "Shader.h"
#include "Buffer.h"
#include "Framebuffer.h"
#include "PipelineLayout.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
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
        std::array<VkDynamicState, 9> dynamic{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
            VK_DYNAMIC_STATE_DEPTH_BIAS,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_DEPTH_BOUNDS,
            VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
            VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE
        };

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        std::vector<VkVertexInputBindingDescription> vertexInputBidings;

        State state;

        Attachment attament;
    };

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> &shader);

    virtual ~Pipeline();

    virtual void Set(std::shared_ptr<Buffer::Super> &buffer) override;
    
    virtual void Set(const InputElementDescription &description) override;

    template <Buffer::Type type>
    std::shared_ptr<Buffer> Get()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.vertexBuffers[0]);
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.indexBuffer);
        }
    }

    VkPipelineBindPoint &BindPoint()
    {
        return bindPoint;
    }

    virtual void Create(std::shared_ptr<Framebuffer::Super> &framebuffer) override;

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

    std::shared_ptr<Framebuffer> framebuffer;

    VkPipeline handle{ VK_NULL_HANDLE };

    VkPipelineCache cache{ VK_NULL_HANDLE };

    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::unique_ptr<PipelineLayout> layout;

    std::unique_ptr<Configuration> configuration;
};

}
}
