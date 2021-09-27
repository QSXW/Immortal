#pragma once

#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class Swapchain
{
public:
    using Description = DXGI_SWAP_CHAIN_DESC;

public:
    Swapchain(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue, Description &desc)
    {
        Check(factory->CreateSwapChain(queue.Get(), &desc, &handle));
    }

    ComPtr<IDXGISwapChain> Handle()
    {
        return handle;
    }

private:
    ComPtr<IDXGISwapChain> handle{ nullptr };
};

}
}
