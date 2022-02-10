#pragma once

#include "Common.h"

#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "PipelineLayout.h"
#include "Descriptor.h"
#include "DescriptorPool.h"
#include "Texture.h"
#include "Barrier.h"

#include <queue>

namespace Immortal
{
namespace Vulkan
{

class Device;
class CommandBuffer;
class GraphicsPipeline : public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

    struct State
    {
        VkPipelineVertexInputStateCreateInfo vertexInput;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly;

        VkPipelineRasterizationStateCreateInfo rasterization;

        VkPipelineColorBlendStateCreateInfo colorBlend;

        VkPipelineDepthStencilStateCreateInfo depthStencil;

        VkPipelineViewportStateCreateInfo viewport;

        VkPipelineMultisampleStateCreateInfo multiSample;

        VkPipelineDynamicStateCreateInfo dynamic;

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;

        std::vector<VkVertexInputBindingDescription> vertexInputBidings;
    };

    struct DescriptorSetPack
    {
        uint32_t Sync = 0;

        VkDescriptorSet DescriptorSets[6];
    };

public:
    GraphicsPipeline(Device *device, std::shared_ptr<Shader::Super> &shader);

    virtual ~GraphicsPipeline();

    virtual void Set(std::shared_ptr<Buffer::Super> &buffer) override;
    
    virtual void Set(const InputElementDescription &description) override;

    virtual void Create(const std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Reconstruct(const std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Bind(const Descriptor *descriptors, uint32_t slot) override;

    virtual void Bind(Texture::Super *superTexture, uint32_t slot) override;

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual void AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

    virtual void CopyState(Super &other) override;

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
	    state->inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    state->inputAssembly.topology               = ConvertType(desc.PrimitiveType);
	    state->inputAssembly.flags                  = 0;
	    state->inputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    void SetupLayout()
    {
        auto &inputAttributeDescriptions = state->inputAttributeDescriptions;
        auto &vertexInputBidings         = state->vertexInputBidings;
        
        state->vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        state->vertexInput.vertexBindingDescriptionCount   = U32(vertexInputBidings.size());
        state->vertexInput.pVertexBindingDescriptions      = vertexInputBidings.data();
        state->vertexInput.vertexAttributeDescriptionCount = U32(inputAttributeDescriptions.size());
        state->vertexInput.pVertexAttributeDescriptions    = inputAttributeDescriptions.data();
    }

private:
    bool Ready();

private:
    Device *device{ nullptr };

    VkPipeline handle{ VK_NULL_HANDLE };

    VkPipelineCache cache{ VK_NULL_HANDLE };

    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::unique_ptr<PipelineLayout> layout;

    std::unique_ptr<State> state;

    std::unique_ptr<DescriptorPool> descriptorPool;

    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    std::unordered_map<uint64_t, DescriptorSetPack> descriptorSets;

    std::queue<DescriptorSetPack> freeDescriptorSetPacks;

    DescriptorSetUpdater *descriptorSetUpdater{ nullptr };
};

class ComputePipeline : public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

    static constexpr VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

public:
    ComputePipeline(Device *device, Shader::Super *shader);

    virtual ~ComputePipeline();

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual void Bind(Texture::Super *superTexture, uint32_t slot) override;

    virtual void Bind(const Descriptor *descriptors, uint32_t slot) override;

    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset) override;

    virtual void ResetResource() override;

    void Dispatch(CommandBuffer *cmdbuf, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ);

    const VkPipelineLayout &Layout() const
    {
        return layout;
    }

    const VkDescriptorSet &GetDescriptorSet() const
    {
        return descriptorSet;
    }

private:
    bool Ready();

private:
    Device *device{ nullptr };

    VkPipeline handle{ VK_NULL_HANDLE };

    VkPipelineCache cache{ VK_NULL_HANDLE };

    VkPipelineLayout layout{ VK_NULL_HANDLE };

    DescriptorSetUpdater *descriptorSetUpdater{ nullptr };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

    std::unique_ptr<DescriptorPool> descriptorPool;

    std::vector<ImageBarrier> barriers;

    uint32_t isChanged = 0;
};

}
}
