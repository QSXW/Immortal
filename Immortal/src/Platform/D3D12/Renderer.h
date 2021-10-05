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

    virtual void PrepareFrame() override
    { 
        frameIndex = swapchain->AcquireCurrentBackBufferIndex();
        
        commandAllocators[frameIndex]->Reset();

        commandList->Reset(commandAllocators[frameIndex]);
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

        frameIndex = Swapchain::SWAP_CHAIN_BUFFER_COUNT - 1;

        for (int i = 0; i < Swapchain::SWAP_CHAIN_BUFFER_COUNT; i++)
        {
            fenceValues[i] = queue->FenceValue();
        }
    }

public:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    Queue *queue{ nullptr };

    CommandList *commandList{ nullptr };

    UINT frameIndex{ Swapchain::SWAP_CHAIN_BUFFER_COUNT - 1 };

    UINT64 fenceValues[Swapchain::SWAP_CHAIN_BUFFER_COUNT]{ 0 };

    ID3D12CommandAllocator **commandAllocators;
};

}
}
