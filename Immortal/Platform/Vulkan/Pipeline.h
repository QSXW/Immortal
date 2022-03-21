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

class Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct DescriptorSetPack
    {
        static constexpr size_t Size = 6;

        uint32_t Sync = 0;

        VkDescriptorSet DescriptorSets[Size];
    };

public:
    Pipeline(Device *device, Shader *shader);

    virtual void Bind(const Descriptor *descriptors, uint32_t slot = 0) override;

    virtual void Bind(Texture::Super *texture, uint32_t slot = 0) override;

    virtual void Bind(Buffer::Super *buffer, uint32_t slot = 0) override;

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

public:
    bool Ready();

    void Update();

protected:
    Device *device{ nullptr };

    VkPipeline handle{ VK_NULL_HANDLE };

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
    GraphicsPipeline(Device *device, std::shared_ptr<Shader::Super> &shader);

    virtual ~GraphicsPipeline();

    virtual void Set(std::shared_ptr<Buffer::Super> &buffer) override;
    
    virtual void Set(const InputElementDescription &description) override;

    virtual void Create(const std::shared_ptr<SuperRenderTarget> &renderTarget, Option option = Option{}) override;

    virtual void Reconstruct(const std::shared_ptr<SuperRenderTarget> &renderTarget, Option option = Option{}) override;

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

    VkDescriptorSet GetDescriptorSet() const
    {
        return descriptor.set;
    }

    operator VkPipeline() const
    {
        return handle;
    }

    VkPipelineLayout Layout() const
    {
        return layout;
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

    virtual void Bind(Texture::Super *superTexture, uint32_t slot) override;

    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset) override;

    virtual void ResetResource() override;

    void Dispatch(CommandBuffer *cmdbuf, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ);

    VkPipelineLayout Layout() const
    {
        return layout;
    }

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
