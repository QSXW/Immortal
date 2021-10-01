#include "impch.h"
#include "GuiLayer.h"

#ifndef _UNICODE
#define _UNICODE
#endif
#include "backends/imgui_impl_win32.cpp"
#include "backends/imgui_impl_dx12.cpp"

#include "Barrier.h"

namespace Immortal
{
namespace D3D12
{

GuiLayer::GuiLayer(SuperRenderContext *superContext) :
    context{ dcast<RenderContext*>(superContext)}
{
    swapchain        = context->Get<Swapchain *>();
    commandList      = context->Get<CommandList *>();
    commandAllocator = context->Get<CommandAllocator *>();
    queue            = context->Get<Queue *>();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    srvDescriptorHeap = context->ShaderResourceViewDescritorHeap();

    ImGui_ImplWin32_Init(context->Get<HWND>());

    ImGui_ImplDX12_Init(
        context->Get<ID3D12Device *>(),
        context->FrameSize(),
        context->Get<DXGI_FORMAT>(),
        srvDescriptorHeap->Handle(),
        srvDescriptorHeap->Get<D3D12_CPU_DESCRIPTOR_HANDLE>(),
        srvDescriptorHeap->Get<D3D12_GPU_DESCRIPTOR_HANDLE>()
        );
}

void GuiLayer::Begin()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    Super::Begin();
}

void GuiLayer::OnGuiRender()
{

}

void GuiLayer::End()
{
    Super::End();
    
    auto &io = ImGui::GetIO();

    Vector2 extent = context->Extent();

    io.DisplaySize = ImVec2{ extent.x, extent.y };

    UINT backBufferIdx = swapchain->AcquireCurrentBackBufferIndex();

    ID3D12Resource *renderTarget = context->RenderTarget(backBufferIdx);

    Barrier barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = renderTarget;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->Reset(commandAllocator);
    commandList->ResourceBarrier(&barrier);

    // Render Dear ImGui graphics
    Descriptor rtvDescritor = std::move(context->RenderTargetDescriptor(backBufferIdx));

    commandList->ClearRenderTargetView(rtvDescritor, rcast<float *>(&clearColor));
    commandList->OMSetRenderTargets(&rtvDescritor, 1, false, nullptr);
    commandList->SetDescriptorHeaps(srvDescriptorHeap, 1);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->Handle());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;

    commandList->ResourceBarrier(&barrier);
    commandList->Close();

    queue->ExecuteCommandLists(commandList);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, rcast<void*>(commandList->Handle()));
    }
}

}
}
