#include "Renderer.h"

namespace Immortal
{
namespace D3D12
{

Renderer::Renderer(RenderContext::Super *superContext)
{
    context = dcast<RenderContext *>(superContext);

    swapchain = context->Get<Swapchain*>();

    INIT();
}

void Renderer::INIT()
{

}

void Renderer::SwapBuffers()
{
    swapchain->Present(1, 0);
    
    currentBuffer = context->WaitForPreviousFrame();
}

}
}