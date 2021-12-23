#include "impch.h"
#include "GuiLayer.h"

#ifndef _UNICODE
#define _UNICODE
#endif
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.cpp>
#include <backends/imgui_impl_glfw.h>

#include <GLFW/glfw3.h>

#include "Render/Render.h"
#include "Barrier.h"
#include "Event/ApplicationEvent.h"

namespace Immortal
{
namespace D3D12
{

GuiLayer::GuiLayer(SuperRenderContext *superContext) :
    context{ dcast<RenderContext*>(superContext)}
{
    swapchain   = context->GetAddress<Swapchain>();
    commandList = context->GetAddress<CommandList>();
    queue       = context->GetAddress<Queue>();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    auto &io       = ImGui::GetIO();
    Vector2 extent = context->Extent();
    io.DisplaySize = ImVec2{ extent.x, extent.y };

    auto window = context->GetAddress<Window>();
    ImGui_ImplWin32_Init(rcast<HWND>(window->Primitive()));

    srvDescriptorHeap     = context->ShaderResourceViewDescritorHeap();
    Descriptor descriptor = context->AllocatorTextureDescriptor();

    ImGui_ImplDX12_Init(
        *context->GetAddress<Device>(),
         context->FrameSize(),
         context->Get<DXGI_FORMAT>(),
         srvDescriptorHeap->Handle(),
         descriptor.cpu,
         descriptor.gpu
        );
}

void GuiLayer::Begin()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    Super::Begin();
}

void GuiLayer::OnEvent(Event &e)
{
    if (e.GetType() == Event::Type::WindowResize)
    {
        auto resize = dcast<WindowResizeEvent *>(&e);
        ImGuiIO   &io  = ImGui::GetIO();
        io.DisplaySize = ImVec2{ (float)resize->Width(), (float)resize->Height() };
    }
    Super::OnEvent(e);
}

void GuiLayer::OnGuiRender()
{
    Super::OnGuiRender();
}

void GuiLayer::End()
{
    Super::End();

    UINT backBufferIdx = Render::CurrentPresentedFrameIndex();

    Barrier barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = context->RenderTarget(backBufferIdx);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->ResourceBarrier(&barrier);

    CPUDescriptor rtvDescritor = std::move(context->RenderTargetDescriptor(backBufferIdx));

    commandList->ClearRenderTargetView(rtvDescritor, rcast<float *>(&clearColor));
    commandList->OMSetRenderTargets(&rtvDescritor, 1, false, nullptr);
    commandList->SetDescriptorHeaps(srvDescriptorHeap->AddressOf(), 1);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->Handle());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;

    commandList->ResourceBarrier(&barrier);

    auto &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, rcast<void*>(commandList->Handle()));
    }
}

}
}
