#include "Renderer.h"

namespace Immortal
{
namespace D3D12
{

Renderer::Renderer(RenderContext::Super *superContext)
{
    context = dcast<RenderContext *>(superContext);

    swapchain = context->Get<Swapchain*>();

    queue = context->Get<Queue *>();

    commandList = context->Get<CommandList *>();

    commandAllocators = context->Get<ID3D12CommandAllocator **>();

    Setup();
}

void Renderer::Setup()
{

}

void Renderer::SwapBuffers()
{
    commandList->Close();
    queue->Handle()->ExecuteCommandLists(1, (ID3D12CommandList **)commandList->AddressOf());

    swapchain->Present(1, 0);

    frameIndex = context->WaitForPreviousFrame();
}

}
}
