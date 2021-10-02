#pragma once

#include "Render/Renderer.h"

#include "D3D12Common.h"
#include "RenderContext.h"

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

    virtual void INIT() override;

    virtual void SwapBuffers() override;

    virtual void RenderFrame() override { }

    virtual uint32_t Index() override
    {
        return currentBuffer;
    }

    virtual void OnResize(UINT x, UINT y, UINT width, UINT height) override
    {
       context->CleanUpRenderTarget();

       swapchain->ResizeBuffers(
            width,
            height,
            DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
        );

       context->CreateRenderTarget();
       currentBuffer = swapchain->AcquireCurrentBackBufferIndex();
    }

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    UINT currentBuffer{ 0 };
};

}
}
