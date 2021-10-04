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

    INIT();
}

void Renderer::INIT()
{

}

void Renderer::SwapBuffers()
{
    auto fenceValue = queue->ExecuteCommandLists(commandList);

    swapchain->Present(1, 0);

    queue->WaitForFence(fenceValue);
}

}
}
