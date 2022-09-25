#include "GuiLayer.h"

#ifndef _UNICODE
#define _UNICODE
#endif
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.cpp>

#include "Render/Render.h"
#include "Event/ApplicationEvent.h"

namespace Immortal
{
namespace D3D11
{

GuiLayer::GuiLayer(SuperRenderContext *superContext) :
    context{ dcast<RenderContext*>(superContext)}
{

}

GuiLayer::~GuiLayer()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
}

void GuiLayer::OnAttach()
{
    Super::OnAttach();

    auto window = context->GetAddress<Window>();
    ImGui_ImplWin32_Init(rcast<HWND>(window->Primitive()));

    auto device = context->GetAddress<Device>();
	ImGui_ImplDX11_Init(*device, device->GetContext());
}

void GuiLayer::Begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    Super::__Begin();
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

void GuiLayer::End()
{
    Super::__End();

	DXGI_SWAP_CHAIN_DESC desc;
	auto swapchain = context->GetAddress<Swapchain>();
	swapchain->GetDesc(&desc);

    auto &io = ImGui::GetIO();
	io.DisplaySize = { (float)desc.BufferDesc.Width, (float)desc.BufferDesc.Height};

    context->Submit([this](CommandList *cmdlist) {
		RenderTarget *renderTarget = context->GetAddress<RenderTarget>();
		cmdlist->OMSetRenderTargets(1, renderTarget->GetViews(), nullptr);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	});

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, nullptr);
    }
}

}
}
