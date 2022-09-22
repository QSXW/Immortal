#include "Swapchain.h"
#include "Device.h"
#include "DescriptorHeap.h"
#include "Queue.h"
#include "RenderContext.h"
#include "Framework/Window.h"

namespace Immortal
{
namespace D3D12
{

Swapchain::Swapchain(RenderContext *context, Queue *queue, Window *window, const DXGI_SWAP_CHAIN_DESC1 &desc) :
    Swapchain{ context, queue, (HWND) window->Primitive(), desc}
{

}

Swapchain::Swapchain(RenderContext *context, Queue *queue, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 &desc) :
    context{context}
{
	auto device = context->GetAddress<Device>();
    auto factory = device->GetAddress<IDXGIFactory4>();
    ComPtr<IDXGISwapChain1> swapchain1;
    Check(factory->CreateSwapChainForHwnd(
        *queue,
        hWnd,
        &desc,
        nullptr,
        nullptr,
        &swapchain1
    ));

    Check(swapchain1.As(&handle));

    renderTargets.resize(desc.BufferCount);
    DescriptorHeap::Description rtvDesc = {
        DescriptorHeap::Type::RenderTargetView,
        desc.BufferCount,
        DescriptorHeap::Flag::None,
        1
    };

    rtvDescriptorHeap = new DescriptorHeap{ device, &rtvDesc };
    CreateRenderTarget();
}

Swapchain::~Swapchain()
{
    ClearRenderTarget();
    handle.Reset();
}

void Swapchain::CreateRenderTarget()
{
    D3D12_RENDER_TARGET_VIEW_DESC *desc = nullptr;
    auto descriptor = rtvDescriptorHeap->StartOfCPU();

    auto device = context->GetAddress<Device>();
    for (int i = 0; i < renderTargets.size(); i++)
    {
        AccessBackBuffer(i, &renderTargets[i]);
        device->CreateView(renderTargets[i], desc, descriptor);
        descriptor.Offset(rtvDescriptorHeap->GetIncrement());
        renderTargets[i]->SetName((std::wstring(L"Swapchain::Render Target{") + std::to_wstring(i) + std::wstring(L"}")).c_str());
    }
}

void Swapchain::ClearRenderTarget()
{
    for (auto &r : renderTargets)
    {
		r->Release();
		r = nullptr;
    }
}

}
}
