#include "Renderer.h"

#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

Renderer::Renderer(RenderContext::Super *superContext) :
    context { dcast<RenderContext *>(superContext) }
{
    swapchain         = context->GetAddress<Swapchain>();
    queue             = context->GetAddress<Queue>();
    commandList       = context->GetAddress<CommandList>();
    commandAllocators = context->GetAddress<ID3D12CommandAllocator*>();
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


Descriptor *Renderer::CreateImageDescriptor(uint32_t count)
{
    auto descriptor = new GPUDescriptor[count];
    CleanUpObject(descriptor);

    return descriptor;
}

Descriptor *Renderer::CreateBufferDescriptor(uint32_t count)
{
    return CreateImageDescriptor(count);
}

}
}
