#pragma once

#include "Common.h"

#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "PipelineLayout.h"
#include "Descriptor.h"

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

    struct Configuration
    {
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

    virtual void Create(std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Reconstruct(std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Bind(std::shared_ptr<SuperTexture> &texture, uint32_t slot) override;

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

    operator VkPipeline&()
    {
        return handle;
    }

    operator VkPipeline() const
    {
        return handle;
    }

    const VkPipelineBindPoint &BindPoint() const
    {
        return bindPoint;
    }

    const VkPipelineLayout &Layout() const
    {
        return std::dynamic_pointer_cast<Shader>(desc.shader)->Get<PipelineLayout&>();
    }

    const VkDescriptorSet &GetDescriptorSet() const
    {
        return descriptorSet;
    }

private:
    VkPrimitiveTopology ConvertType(PrimitiveType &type)
    {
        if (type == PrimitiveType::Line)
        {
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    void SetupVertex()
    {
        auto state = &configuration->state;
	    state->inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    state->inputAssembly.topology               = ConvertType(desc.PrimitiveType);
	    state->inputAssembly.flags                  = 0;
	    state->inputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    void SetupLayout()
    {
        auto state = &configuration->state;
        auto &inputAttributeDescriptions = configuration->inputAttributeDescriptions;
        auto &vertexInputBidings         = configuration->vertexInputBidings;
        
        state->vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        state->vertexInput.vertexBindingDescriptionCount   = U32(vertexInputBidings.size());
        state->vertexInput.pVertexBindingDescriptions      = vertexInputBidings.data();
        state->vertexInput.vertexAttributeDescriptionCount = U32(inputAttributeDescriptions.size());
        state->vertexInput.pVertexAttributeDescriptions    = inputAttributeDescriptions.data();
    }

    bool Ready()
    {
        return descriptorSetUpdater->Ready();
    }

private:
    Device *device{ nullptr };

    VkPipeline handle{ VK_NULL_HANDLE };

    VkPipelineCache cache{ VK_NULL_HANDLE };

    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::unique_ptr<PipelineLayout> layout;

    std::unique_ptr<Configuration> configuration;

    VkDescriptorSet descriptorSet;

    DescriptorSetUpdater *descriptorSetUpdater{ nullptr };
};

}
}
