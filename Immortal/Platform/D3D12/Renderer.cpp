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
    queue->Execute(commandList->AddressOf<ID3D12CommandList>());

    swapchain->Present(1, 0);

    context->WaitForPreviousFrame();
}

Descriptor::Super *Renderer::CreateImageDescriptor(uint32_t count)
{
    auto descriptor = new CPUDescriptor[count];
    CleanUpObject(descriptor);

    return descriptor;
}

Descriptor::Super *Renderer::CreateBufferDescriptor(uint32_t count)
{
    return CreateImageDescriptor(count);
}

void Renderer::Draw(GraphicsPipeline::Super *super)
{
    auto pipeline       = dynamic_cast<GraphicsPipeline *>(super);
    auto indexBuffer    = pipeline->GetBuffer<Buffer::Type::Index>();
    auto descriptorHeap = pipeline->GetAddress<DescriptorHeap>();

    commandList->SetPipelineState(*pipeline);
    commandList->SetGraphicsRootSignature(pipeline->Get<RootSignature&>());
    commandList->SetDescriptorHeaps(descriptorHeap->AddressOf(), 1);
    commandList->SetPrimitiveTopology(pipeline->Get<D3D12_PRIMITIVE_TOPOLOGY>());
    commandList->SetVertexBuffers(&pipeline->Get<Buffer::VertexView>());
    commandList->SetIndexBuffer(&pipeline->Get<Buffer::IndexView>());

    GPUDescriptor descriptors{ descriptorHeap->StartOfGPU() };
    auto &descriptorTables = pipeline->GetDescriptorTables();
    for (uint32_t i = 0; i < descriptorTables.size(); i++)
    {
        GPUDescriptor descriptor = descriptors;
        descriptor.Offset(descriptorTables[i].Offset, descriptorHeap->GetIncrement());
        commandList->SetGraphicsRootDescriptorTable(i + 1 /* 0 is reserved by root constants */, descriptor);
    }

    commandList->DrawIndexedInstance(pipeline->ElementCount, 1, 0, 0, 0);
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

    auto &colorBuffers = renderTarget->GetColorBuffers();
    auto &depthBuffer  = renderTarget->GetDepthBuffer();

    for (auto &colorBuffer : colorBuffers)
    {
        barriers[activeBarrier++].Transition(
            colorBuffer,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
    }

    barriers[activeBarrier].Transition(
        depthBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );
    activeBarrier++;

    commandList->ResourceBarrier(barriers.data(), activeBarrier);

    const auto &rtvDescriptor = renderTarget->GetDescriptor();
    const auto &dsvDescriptor = depthBuffer.GetDescriptor();
    commandList->ClearRenderTargetView(rtvDescriptor[0], rcast<float *>(renderTarget->ClearColor()));
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);
    commandList->SetRenderTargets(rtvDescriptor, U32(colorBuffers.size()), false, &dsvDescriptor);
}

void Renderer::End()
{
    for (auto &barrier : barriers)
    {
        barrier.Swap();
    }
    commandList->ResourceBarrier(barriers.data(), activeBarrier);
    activeBarrier = 0;
}

void Renderer::PushConstant(Pipeline::Super *super, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset)
{
    (void)stage;

    auto pipeline = dynamic_cast<Pipeline *>(super);
    commandList->SetGraphicsRootSignature(pipeline->Get<RootSignature&>());
    commandList->PushConstant(size, data, offset);
}

}
}
