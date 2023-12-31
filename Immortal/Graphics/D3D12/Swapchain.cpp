#include "Swapchain.h"
#include "Device.h"
#include "DescriptorHeap.h"
#include "Queue.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Window.h"

namespace Immortal
{
namespace D3D12
{

Swapchain::Swapchain() :
    NonDispatchableHandle{},
    renderTargets{},
    bufferIndex{}
{

}

Swapchain::Swapchain(Device *device, Queue *queue, Window *window, const DXGI_SWAP_CHAIN_DESC1 &desc, SwapchainMode mode) :
    Swapchain{ device, queue, (HWND)window->GetBackendHandle(), desc, mode }
{

}

Swapchain::Swapchain(Device *device, Queue *queue, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 &desc, SwapchainMode swapchainMode) :
    NonDispatchableHandle{ device }
{
    mode = swapchainMode;

    auto dxgiFactory = device->GetDXGIFactory();
    ComPtr<IDXGISwapChain1> swapchain1;
    Check(dxgiFactory->CreateSwapChainForHwnd(
        *queue,
        hWnd,
        &desc,
        nullptr,
        nullptr,
        &swapchain1
    ));

    Check(swapchain1.As(&handle));

    renderTargets.resize(desc.BufferCount);
    CreateRenderTarget();

    Check(dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
}

Swapchain::~Swapchain()
{
    ClearRenderTarget();
    handle.Reset();
}

void Swapchain::CreateRenderTarget()
{
    for (int i = 0; i < renderTargets.size(); i++)
    {
        ComPtr<ID3D12Resource> resource = nullptr;
        handle->GetBuffer(i, IID_PPV_ARGS(&resource));

#ifdef _DEBUG
        std::wstring name = L"Swapchain::RenderTarget" + std::to_wstring(i);
        Check(resource->SetName(name.c_str()));
#endif

        Ref<Texture> texture = new Texture{ device, resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET};
        renderTargets[i] = new RenderTarget{  device };
        renderTargets[i]->SetColorAttachment(0, texture);
    }
}

void Swapchain::PrepareNextFrame()
{
    AcquireCurrentBackBufferIndex();
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
    DXGI_SWAP_CHAIN_DESC desc{};
    Check(handle->GetDesc(&desc));

    ClearRenderTarget();
    Check(handle->ResizeBuffers(desc.BufferCount, width, height, DXGI_FORMAT_UNKNOWN, desc.Flags));
    CreateRenderTarget();
}

SuperRenderTarget *Swapchain::GetCurrentRenderTarget()
{
    return renderTargets[bufferIndex];
}

void Swapchain::ClearRenderTarget()
{
    for (auto &r : renderTargets)
    {
        r.Reset();
        r = nullptr;
    }
}

}
}
