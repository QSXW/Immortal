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
#include "Barrier.h"

#include <queue>

namespace Immortal
{
namespace Vulkan
{

class Device;
class CommandBuffer;

class Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct DescriptorSetPack
    {
        static constexpr size_t Size = 3;

        uint32_t Sync = 0;

        VkDescriptorSet DescriptorSets[Size];
    };

    using Primitive = VkPipeline;
    VKCPP_OPERATOR_HANDLE()

public:
    Pipeline(Device *device, Shader *shader);

    virtual ~Pipeline();

    virtual void Bind(const DescriptorBuffer *descriptorBuffer, uint32_t slot = 0) override;

    virtual void Bind(SuperTexture *texture, uint32_t slot = 0) override;

    virtual void Bind(Buffer::Super *buffer, uint32_t slot = 0) override;

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

public:
	void Bind(const Descriptor *descriptor, uint32_t slot);

    bool Ready();

    void Update();

    void Destroy();

    VkPipelineLayout Layout() const
	{
		return layout;
	}

protected:
    Device *device{ nullptr };

    VkPipelineCache cache{ VK_NULL_HANDLE };

    PipelineLayout layout;

    struct
    {
        VkDescriptorSetLayout setLayout;

        VkDescriptorSet set{ VK_NULL_HANDLE };

        DescriptorSetUpdater *setUpdater{ nullptr };

        std::unique_ptr<DescriptorPool> pool;

        std::unordered_map<uint64_t, DescriptorSetPack> packs;

        std::queue<DescriptorSetPack> freePacks;
    } descriptor;
};

class GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline
{
public:
    using NativeSuper = Vulkan::Pipeline;

    using Super = SuperGraphicsPipeline;

    static constexpr VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

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

public:
    GraphicsPipeline(Device *device, Ref<Shader::Super> shader);

    GraphicsPipeline(Device *device, Shader *shader);

    virtual ~GraphicsPipeline();

    virtual void Set(const InputElementDescription &description) override;

    virtual void Create(const SuperRenderTarget *renderTarget) override;

    virtual void Reconstruct(const SuperRenderTarget *renderTarget) override;

    virtual void CopyState(Super &other) override;

    template <Buffer::Type type>
    Ref<Buffer> Get()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return dynamic_cast<Buffer *>(desc.vertexBuffers[0].Get());
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return dynamic_cast<Buffer *>(desc.indexBuffer.Get());
        }
    }

    VkDescriptorSet GetDescriptorSet() const
    {
        return descriptor.set;
    }

private:
    void SetupVertex();

    void SetupLayout();

private:
    std::unique_ptr<State> state;
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using NativeSuper = Vulkan::Pipeline;

    using Super = SuperComputePipeline;

    static constexpr VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

public:
    ComputePipeline(Device *device, Shader::Super *shader);

    virtual ~ComputePipeline();

    virtual void Bind(SuperTexture *texture, uint32_t slot) override;

    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset) override;

    void Dispatch(CommandBuffer *cmdbuf, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ);

    const VkDescriptorSet &GetDescriptorSet() const
    {
        return descriptor.set;
    }

private:
    void Update();

private:
    std::vector<ImageBarrier> barriers;
};

}
}
