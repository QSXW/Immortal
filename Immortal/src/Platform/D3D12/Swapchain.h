#pragma once

#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class Swapchain
{
public:
    using Description = DXGI_SWAP_CHAIN_DESC1;

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
    }

    ComPtr<IDXGISwapChain1> Handle()
    {
        return handle;
    }

private:
    ComPtr<IDXGISwapChain1> handle{ nullptr };

    ComPtr<IDXGISwapChain3> handle3{ nullptr };
};

}
}
