#pragma once

#include "Render/Renderer.h"

#include "Common.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "Texture.h"
#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D12
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

public:
    Renderer(RenderContext::Super *context);

    virtual void Setup() override;

    virtual void SwapBuffers() override;

    virtual void PrepareFrame() override;

    virtual const char *GraphicsRenderer() override
    {
        return context->GraphicsRenderer();
    }

    virtual void OnResize(UINT x, UINT y, UINT width, UINT height) override
    {
        context->UpdateSwapchain(width, height);
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override
    {
        return new Buffer{ context->GetAddress<Device>(), size, data, type };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, Buffer::Type type) override
    {
        return new Buffer{ context->GetAddress<Device>(), size, type };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, uint32_t binding) override
    {
        return new Buffer{ context->GetAddress<Device>(), size, binding };
    }

    virtual Shader::Super *CreateShader(const std::string &filepath, Shader::Type type) override
    {
        return new Shader{ filepath, type };
    }

    virtual GraphicsPipeline::Super *CreateGraphicsPipeline(std::shared_ptr<Shader::Super> &shader) override
    {
        return new GraphicsPipeline{ context->GetAddress<Device>(), shader };
    }

    virtual ComputePipeline::Super *CreateComputePipeline(Shader::Super *shader) override
    {
        return new ComputePipeline{ context->GetAddress<Device>(), shader };
    }

    virtual Texture::Super *CreateTexture(const std::string &filepath, const Texture::Description &description = {}) override
    {
        return new Texture{ context, filepath, description };
    }

    virtual Texture::Super *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override
    {
        return new Texture{ context, width, height, data, description };
    }

    virtual RenderTarget::Super *CreateRenderTarget(const RenderTarget::Description &description) override
    {
        return new RenderTarget{ context->GetAddress<Device>(), description };
    }

    virtual Descriptor::Super *CreateImageDescriptor(uint32_t count) override;

    virtual Descriptor::Super *CreateBufferDescriptor(uint32_t count) override;

    virtual void Draw(GraphicsPipeline::Super *pipeline) override;

    virtual void Begin(std::shared_ptr<RenderTarget::Super> &renderTarget) override;

    virtual void End() override;

    void PushConstant(Pipeline::Super *super, Shader::Stage stage, uint32_t size, const void * data, uint32_t offset);

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    Queue *queue{ nullptr };

    CommandList *commandList{ nullptr };

    ID3D12CommandAllocator **commandAllocators;

    std::array<Barrier<BarrierType::Transition>, 8> barriers;

    uint32_t frameIndex = 0;

    uint32_t activeBarrier = 0;
};

}
}
