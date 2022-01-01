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

    virtual uint32_t Index() override
    {
        return frameIndex;
    }

    virtual const char *GraphicsRenderer() override
    {
        return context->GraphicsRenderer();
    }

    virtual void OnResize(UINT x, UINT y, UINT width, UINT height) override
    {
        context->UpdateSwapchain(width, height);

        frameIndex = Swapchain::SWAP_CHAIN_BUFFER_COUNT - 1;

        for (int i = 0; i < Swapchain::SWAP_CHAIN_BUFFER_COUNT; i++)
        {
            fenceValues[i] = queue->FenceValue();
        }
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

    virtual Pipeline::Super *CreatePipeline(std::shared_ptr<Shader::Super> &shader)
    {
        return new Pipeline{ context->GetAddress<Device>(), shader };
    }

    virtual Texture::Super *CreateTexture(const std::string &filepath) override
    {
        return new Texture{ context, filepath };
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

    virtual void Draw(const std::shared_ptr<Pipeline::Super> &pipeline) override;

    virtual void Begin(std::shared_ptr<RenderTarget::Super> &renderTarget) override;

    virtual void End() override;

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    Queue *queue{ nullptr };

    CommandList *commandList{ nullptr };

    ID3D12CommandAllocator **commandAllocators;

    UINT frameIndex{ Swapchain::SWAP_CHAIN_BUFFER_COUNT - 1 };

    UINT64 fenceValues[Swapchain::SWAP_CHAIN_BUFFER_COUNT]{ 0 };

    Barrier<BarrierType::Transition> barrier;
};

}
}
