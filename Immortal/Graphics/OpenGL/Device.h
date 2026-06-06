#pragma once

#include "Graphics/Device.h"
#include "Graphics/LightGraphics.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Device : public SuperDevice
{
public:
	Device();

	virtual ~Device() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual BackendAPI GetBackendAPI() override;

    virtual SuperQueue *CreateQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) override;

    virtual SuperCommandBuffer *CreateCommandBuffer(QueueType type) override;

    virtual SuperSwapchain *CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) override;

    virtual SuperSampler *CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation = CompareOperation::Never, float minLod = .0f, float maxLod = 1.0f) override;

    virtual SuperShader *CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) override;

    virtual SuperGraphicsPipeline *CreateGraphicsPipeline() override;

    virtual SuperBuffer *CreateBuffer(size_t size, BufferType type) override;

    virtual SuperTexture *CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels = 1, uint16_t arrayLayers = 1, TextureType type = TextureType::None) override;

    virtual SuperDescriptorSet *CreateDescriptorSet(SuperPipeline *pipeline) override;

    virtual SuperGPUEvent *CreateGPUEvent(const std::string &name) override;

    virtual SuperRenderTarget *CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {}) override;

protected:
	bool loaded;
};

}
}
