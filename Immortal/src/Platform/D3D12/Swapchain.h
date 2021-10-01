#pragma once

#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class Swapchain
{
public:
    struct Description : public DXGI_SWAP_CHAIN_DESC1
    {
        
    };

public:
    Swapchain(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue, HWND hWnd, Description &desc)
    {
        Check(factory->CreateSwapChainForHwnd(
            queue.Get(),
            hWnd,
            &desc,
            nullptr,
            nullptr,
            &handle
            ));

        Check(handle.As(&handle3));

        writableObject = handle3->GetFrameLatencyWaitableObject();
    }

    UINT AcquireCurrentBackBufferIndex()
    {
        return handle3->GetCurrentBackBufferIndex();
    }

    void AccessBackBuffer(UINT index, ID3D12Resource **pRenderTarget)
    {
        Check(handle3->GetBuffer(index, IID_PPV_ARGS(pRenderTarget)));
    }

    void SetMaximumFrameLatency(UINT maxLatency)
    {
        Check(handle3->SetMaximumFrameLatency(maxLatency));
    }

    ComPtr<IDXGISwapChain1> Handle()
    {
        return handle;
    }

private:
    ComPtr<IDXGISwapChain1> handle{ nullptr };

    ComPtr<IDXGISwapChain3> handle3{ nullptr };

    HANDLE writableObject{ nullptr };
};

}
}
