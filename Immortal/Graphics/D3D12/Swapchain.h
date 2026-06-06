#pragma once

#include "Common.h"
#include "Resource.h"
#include "Descriptor.h"
#include "DescriptorHeap.h"
#include "Graphics/Swapchain.h"

namespace Immortal
{

class Window;
namespace D3D12
{

class GPUEvent;
class Queue;
class RenderContext;
class Device;
class DescriptorHeap;
class RenderTarget;
class IMMORTAL_API Swapchain : public SuperSwapchain, public NonDispatchableHandle
{
public:
    enum class BitDepth
    {
        _8  = 0,
        _10,
        _16,
        Count
    };

    static inline constexpr UINT64 SWAP_CHAIN_BUFFER_COUNT = 3;

    using Primitive = IDXGISwapChain4;
    D3D_OPERATOR_HANDLE()
    D3D_SWAPPABLE(Swapchain)

public:
    Swapchain();

    Swapchain(Device *device, Queue *queue, Window *window, const DXGI_SWAP_CHAIN_DESC1 &desc, SwapchainMode mode = SwapchainMode::None);

    Swapchain(Device *device, Queue *queue, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 &desc, SwapchainMode mode);

    virtual ~Swapchain() override;

    virtual void PrepareNextFrame() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual SuperRenderTarget *GetCurrentRenderTarget() override;

    void CreateRenderTarget();

    void ClearRenderTarget();

public:
    void Present()
    {
        Present((UINT)mode, 0);
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

    UINT AcquireCurrentBackBufferIndex()
    {
        bufferIndex = handle->GetCurrentBackBufferIndex();
        return bufferIndex;
    }

    uint32_t GetCurrentFrameIndex() const
    {
        return bufferIndex;
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

    void GetDesc(DXGI_SWAP_CHAIN_DESC1 *desc)
    {
        handle->GetDesc1(desc);
    }

    void Swap(Swapchain &other)
    {
        NonDispatchableHandle::Swap(other);
        std::swap(mode,          other.mode         );
        std::swap(renderTargets, other.renderTargets);
        std::swap(bufferIndex,   other.bufferIndex  );
    }

protected:
    std::vector<URef<RenderTarget>> renderTargets;

    uint32_t bufferIndex = 0;
};

}
}
