#pragma once

#include "Common.h"

#include "Graphics/RenderTarget.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "PipelineLayout.h"
#include "Descriptor.h"
#include "DescriptorPool.h"
#include "Barrier.h"
#include "Handle.h"

#include <queue>

namespace Immortal
{
namespace Vulkan
{

class Device;
class CommandBuffer;

class Pipeline : public virtual SuperPipeline, public Handle<VkPipeline>
{
public:
    using Super = SuperPipeline;

    struct DescriptorSetPack
    {
        static constexpr size_t Size = 3;

        uint32_t Sync = 0;

        VkDescriptorSet DescriptorSets[Size];
    };

    VKCPP_SWAPPABLE(Pipeline)

public:
	Pipeline(Device *device = nullptr);

    virtual ~Pipeline() override;

public:
	void ConstructPipelineLayout(const std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, const std::vector<VkPushConstantRange> &pushConstantRanges);

    void Destroy();

    VkPipelineLayout GetPipelineLayout() const
    {
		return pipelineLayout;
    }

    VkDescriptorSetLayout GetDescriptorSetLayout() const
    {
		return descriptorSetLayout;
    }

    VkPipelineBindPoint GetBindPoint() const
    {
		return bindPoint;
    }

    void SetBindPoint(VkPipelineBindPoint value)
    {
		bindPoint = value;
    }

    void Swap(Pipeline &other)
    {
		Handle::Swap(other);
		std::swap(device,              other.device             );
		std::swap(pipelineCache,       other.pipelineCache      );
		std::swap(pipelineLayout,      other.pipelineLayout     );
		std::swap(descriptorSetLayout, other.descriptorSetLayout);
		std::swap(bindPoint,           other.bindPoint          );
    }

protected:
    Device *device{ nullptr };

    VkPipelineCache pipelineCache{VK_NULL_HANDLE};

    PipelineLayout pipelineLayout;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipelineBindPoint bindPoint;

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
    GraphicsPipeline(Device *device);

    GraphicsPipeline(Device *device, Ref<Shader::Super> shader);

    GraphicsPipeline(Device *device, Shader *shader);

    virtual ~GraphicsPipeline() override;

    virtual void Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription) override;
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using NativeSuper = Vulkan::Pipeline;

    using Super = SuperComputePipeline;

    static constexpr VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

public:
	ComputePipeline(Device *device, SuperShader *shader);

    virtual ~ComputePipeline() override;
};

}
}
