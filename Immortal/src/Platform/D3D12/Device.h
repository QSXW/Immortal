#pragma once

#include "Framework/Device.h"
#include "D3D12Common.h"

#include "Queue.h"

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
        return handle.Get();
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

    void CreateRenderTargetView(ComPtr<ID3D12Resource> renderTarget, D3D12_RENDER_TARGET_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        handle->CreateRenderTargetView(renderTarget.Get(), pDesc, descriptor);
    }

private:
    ComPtr<ID3D12Device> handle{ nullptr };

    static inline bool UseWarpDevice{ true };
};

}
}
