#pragma once

#include "Framework/Utils.h"
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

    ~Device()
    {
        IfNotNullThenRelease(handle);
    }

    ID3D12Device *Handle()
    {
        return handle;
    }

    operator const ID3D12Device*() const
    {
        return handle;
    }

    operator ID3D12Device*()
    {
        return handle;
    }

    std::unique_ptr<Queue> CreateQueue(Queue::Description &desc) const
    {
        return std::make_unique<Queue>(handle, desc);
    }

    UINT DescriptorHandleIncrementSize(const D3D12_DESCRIPTOR_HEAP_TYPE &type) const
    {
        return handle->GetDescriptorHandleIncrementSize(
            type
            );
    }

    void CreateRenderTargetView(ID3D12Resource *pRenderTarget, D3D12_RENDER_TARGET_VIEW_DESC *pDesc, CPUDescriptor &descriptor) const
    {
        handle->CreateRenderTargetView(
            pRenderTarget,
            pDesc,
            descriptor
            );
    }

    void CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator **ppAllocator) const
    {
        Check(handle->CreateCommandAllocator(
            type,
            IID_PPV_ARGS(ppAllocator)
            ));
    }

    void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pipeleState, ID3D12GraphicsCommandList **ppCommandList, UINT nodeMask = 0) const
    {
        Check(handle->CreateCommandList(
            nodeMask,
            type,
            pAllocator,
            pipeleState,
            IID_PPV_ARGS(ppCommandList)
            ));
    }

    void CreateFence(ID3D12Fence **pfence, UINT64 initialValue = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE) const
    {
        Check(handle->CreateFence(
            initialValue,
            flag,
            IID_PPV_ARGS(pfence)
            ));
    }

    void CreateCommittedResource(
        const D3D12_HEAP_PROPERTIES *pHeapProperties,
              D3D12_HEAP_FLAGS       heapFlags,
        const D3D12_RESOURCE_DESC   *pDesc,
              D3D12_RESOURCE_STATES  initialResourceState,
        const D3D12_CLEAR_VALUE     *pOptimizedClearValue,
              ID3D12Resource       **pResource) const
    {
        Check(handle->CreateCommittedResource(
            pHeapProperties,
            heapFlags,
            pDesc,
            initialResourceState,
            pOptimizedClearValue,
            IID_PPV_ARGS(pResource)
            ));
    }

    DXGI_ADAPTER_DESC AdaptorDesc();

    void Set(const std::wstring &name)
    {
        handle->SetName(name.c_str());
    }

    void Set(const std::string &name)
    {
        auto wName = Utils::s2ws(name);
        Set(wName);
    }

private:
    ID3D12Device *handle{ nullptr };

    IDXGIFactory4 *dxgiFactory{ nullptr };

    static inline bool UseWarpDevice{ false };
};

}
}
