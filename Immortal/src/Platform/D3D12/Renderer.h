#pragma once

#include "Render/Renderer.h"

#include "D3D12Common.h"
#include "RenderContext.h"

#include <mutex>

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

    virtual void RenderFrame() override
    { 
        frameIndex = swapchain->AcquireCurrentBackBufferIndex();
    }

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
    }

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    Queue *queue{ nullptr };

    CommandList *commandList{ nullptr };

    UINT frameIndex{ 0 };
};

}
}
