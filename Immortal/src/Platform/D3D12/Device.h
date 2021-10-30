#pragma once

#include "Framework/Device.h"
#include "Common.h"

#include "Queue.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device : SuperDevice
{
public:
    using Super = SuperDevice;

public:
    Device(ComPtr<IDXGIFactory4> factory);

    ID3D12Device *Handle()
    {
        return handle;
    }

    std::unique_ptr<Queue> CreateQueue(Queue::Description &desc)
    {
        return std::make_unique<Queue>(handle, desc);
    }

    UINT DescriptorHandleIncrementSize(const D3D12_DESCRIPTOR_HEAP_TYPE &type)
    {
        return handle->GetDescriptorHandleIncrementSize(
            type
            );
    }

    void CreateRenderTargetView(ID3D12Resource *pRenderTarget, D3D12_RENDER_TARGET_VIEW_DESC *pDesc, Descriptor &descriptor)
    {
        handle->CreateRenderTargetView(
            pRenderTarget,
            pDesc,
            descriptor
            );
    }

    void CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator **ppAllocator)
    {
        Check(handle->CreateCommandAllocator(
            type,
            IID_PPV_ARGS(ppAllocator)
            ));
    }

    void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pipeleState, ID3D12GraphicsCommandList **ppCommandList, UINT nodeMask = 0)
    {
        Check(handle->CreateCommandList(
            nodeMask,
            type,
            pAllocator,
            pipeleState,
            IID_PPV_ARGS(ppCommandList)
            ));
    }

    void CreateFence(ID3D12Fence **pfence, UINT64 initialValue = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE)
    {
        handle->CreateFence(
            initialValue,
            flag,
            IID_PPV_ARGS(pfence)
            );
    }

    DXGI_ADAPTER_DESC AdaptorDesc();

private:
    ID3D12Device *handle{ nullptr };

    IDXGIFactory4 *dxgiFactory{ nullptr };

    static inline bool UseWarpDevice{ false };
};

}
}
