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

    virtual const char *GraphicsRenderer() override
    {
        return context->GraphicsRenderer();
    }

    virtual void OnResize(UINT x, UINT y, UINT width, UINT height) override
    {
        currentBuffer = context->UpdateSwapchain(width, height);
    }

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    UINT currentBuffer{ 0 };
};

}
}
