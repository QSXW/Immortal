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

    virtual void SwapBuffers() override { }

    void SubmitFrame() { }

    virtual void RenderFrame() override { }

    virtual uint32_t Index() override
    {
        return currentBuffer;
    }

    void Resize();

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    UINT currentBuffer{ 0 };
};

}
}
