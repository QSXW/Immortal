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

void Renderer::PrepareFrame()
{
    frameIndex = swapchain->AcquireCurrentBackBufferIndex();

    commandAllocators[frameIndex]->Reset();

    commandList->Reset(commandAllocators[frameIndex]);
}

void Renderer::SwapBuffers()
{
    commandList->Close();
    queue->Handle()->ExecuteCommandLists(1, (ID3D12CommandList **)commandList->AddressOf());

    swapchain->Present(1, 0);

    frameIndex = context->WaitForPreviousFrame();
}

Descriptor::Super *Renderer::CreateImageDescriptor(uint32_t count)
{
    auto descriptor = new GPUDescriptor[count];
    CleanUpObject(descriptor);

    return descriptor;
}

Descriptor::Super *Renderer::CreateBufferDescriptor(uint32_t count)
{
    return CreateImageDescriptor(count);
}

void Renderer::Draw(const std::shared_ptr<Pipeline::Super> &superPipeline)
{
    auto pipeline = std::dynamic_pointer_cast<Pipeline>(superPipeline);
    auto indexBuffer = pipeline->GetBuffer<Buffer::Type::Index>();

    auto descriptorHeap = pipeline->GetAddress<DescriptorPool>();

    commandList->SetPipelineState(*pipeline);
    commandList->SetGraphicsRootSignature(pipeline->Get<RootSignature&>());
    commandList->SetDescriptorHeaps(descriptorHeap->AddressOf(), 1);
    commandList->SetGraphicsRootDescriptorTable(0, descriptorHeap->StartOfGPU());
    commandList->SetPrimitiveTopology(pipeline->Get<D3D12_PRIMITIVE_TOPOLOGY>());
    commandList->SetVertexBuffers(&pipeline->Get<Buffer::VertexView>());
    commandList->SetIndexBuffer(&pipeline->Get<Buffer::IndexView>());
    commandList->DrawIndexedInstance(indexBuffer->Count(), 1, 0, 0, 0);
}

void Renderer::Begin(std::shared_ptr<RenderTarget::Super> &superRenderTarget)
{
    auto renderTarget = std::dynamic_pointer_cast<RenderTarget>(superRenderTarget);

    auto width  = renderTarget->Width();
    auto height = renderTarget->Height();

    Viewport viewport{ 0, 0, width, height };
    commandList->RSSetViewports(&viewport);

    Rect scissorRect{ 0, 0, width, height };
    commandList->RSSetScissorRects(&scissorRect);

    barrier.Transition(
        *renderTarget,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(&barrier);

    CPUDescriptor rtvDescriptor = renderTarget->GetDescriptor();
    commandList->ClearRenderTargetView(rtvDescriptor, rcast<float *>(renderTarget->ClearColor()));
    commandList->OMSetRenderTargets(&rtvDescriptor, 1, false, nullptr);
}

void Renderer::End()
{
    barrier.Swap();
    commandList->ResourceBarrier(&barrier);
}

}
}
