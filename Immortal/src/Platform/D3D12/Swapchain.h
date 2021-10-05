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

    enum class BitDepth
    {
        _8  = 0,
        _10,
        _16,
        Count
    };

    static inline constexpr UINT64 SWAP_CHAIN_BUFFER_COUNT = 3;

public:
    Swapchain(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> queue, HWND hWnd, Description &desc)
    {
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
    }

    UINT AcquireCurrentBackBufferIndex()
    {
        return handle->GetCurrentBackBufferIndex();
    }

    void AccessBackBuffer(UINT index, ID3D12Resource **pRenderTarget)
    {
        Check(handle->GetBuffer(index, IID_PPV_ARGS(pRenderTarget)));
    }

    void SetMaximumFrameLatency(UINT maxLatency)
    {
        Check(handle->SetMaximumFrameLatency(maxLatency));
    }

    HANDLE FrameLatencyWaitableObject()
    {
        return handle->GetFrameLatencyWaitableObject();
    }

    ComPtr<IDXGISwapChain1> Handle()
    {
        return handle;
    }

    void Present(UINT syncInterval, UINT flags)
    {
        handle->Present(syncInterval, flags);
    }

    void ResizeBuffers(UINT width, UINT height, DXGI_FORMAT newFormat, UINT flags, UINT bufferCount = 0)
    {
        Check(handle->ResizeBuffers(
            bufferCount,
            width,
            height,
            newFormat,
            flags
            ));
    }

    bool CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE colorSpace, UINT *pSupport)
    {
        return SUCCEEDED(handle->CheckColorSpaceSupport(colorSpace, pSupport));
    }

    void Set(DXGI_COLOR_SPACE_TYPE colorSpace)
    {
        Check(handle->SetColorSpace1(colorSpace));
    }

    void Set(DXGI_HDR_METADATA_TYPE type, UINT size, void *pMetaData)
    {
        Check(handle->SetHDRMetaData(type, size, pMetaData));
    }

    void Desc(DXGI_SWAP_CHAIN_DESC1 *desc)
    {
        handle->GetDesc1(desc);
    }

private:
    ComPtr<IDXGISwapChain4> handle{ nullptr };
};

}
}
