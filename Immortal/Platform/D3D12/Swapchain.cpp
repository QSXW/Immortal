#include "Swapchain.h"
#include "Device.h"
#include "DescriptorHeap.h"

namespace Immortal
{
namespace D3D12
{

Swapchain::Swapchain(Device *device, ComPtr<ID3D12CommandQueue> queue, HWND hWnd, Description &desc) :
    device{ device }
{
    auto factory = device->GetAddress<IDXGIFactory4>();
    ComPtr<IDXGISwapChain1> swapchain1;
    Check(factory->CreateSwapChainForHwnd(
        queue.Get(),
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

    rtvDescriptorHeap.reset(new DescriptorHeap{ *device, &rtvDesc });
    CreateRenderTarget();
}

void Swapchain::CreateRenderTarget()
{
    D3D12_RENDER_TARGET_VIEW_DESC *desc = nullptr;
    auto descriptor = rtvDescriptorHeap->StartOfCPU();

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
