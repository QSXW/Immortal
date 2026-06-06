#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/LightGraphics.h"
#include <unordered_map>

namespace Immortal
{

namespace Metal
{

class PhysicalDevice;
class DescriptorPool;
class DescriptorHeap;
class Sampler;
class Pipeline;
class IMMORTAL_API Device : public SuperDevice, public Handle<MTL::Device>
{
public:
    METAL_SWAPPABLE(Device)

public:
    Device();

	Device(PhysicalDevice *physicalDevice);

    virtual ~Device() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual BackendAPI GetBackendAPI() override;

    virtual SuperQueue *CreateQueue(QueueType type, QueuePriority priority) override;

    virtual SuperCommandBuffer *CreateCommandBuffer(QueueType type) override;

    virtual SuperSwapchain *CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) override;

    virtual SuperSampler *CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) override;

    virtual SuperShader *CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) override;

    virtual SuperGraphicsPipeline *CreateGraphicsPipeline() override;

    virtual SuperComputePipeline *CreateComputePipeline(SuperShader *shader) override;

    virtual SuperTexture *CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) override;

    virtual SuperBuffer *CreateBuffer(size_t size, BufferType type) override;

    virtual SuperDescriptorSet *CreateDescriptorSet(SuperPipeline *pipeline) override;

    virtual SuperGPUEvent *CreateGPUEvent(const std::string &name) override;

    virtual SuperRenderTarget *CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {}) override;

public:
    MTL::CommandQueue *GetCommandQueue(QueueType type);

public:
    void Swap(Device &other)
    {
        Handle::Swap(other);
        std::swap(physicalDevice, other.physicalDevice);
        std::swap(queues,         other.queues        );
    }

protected:
	PhysicalDevice *physicalDevice;

    std::unordered_map<QueueType, MTL::CommandQueue *> queues;
};

}
}
