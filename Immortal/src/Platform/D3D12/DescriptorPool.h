#pragma once
#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class DescriptorPool
{
public:
    enum class Type
    {
        RenderTargetView   = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        ShaderResourceView = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    };

    enum class Flag
    {
        None          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        ShaderVisible = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
    };

    struct Description : public D3D12_DESCRIPTOR_HEAP_DESC
    {
        using Super = D3D12_DESCRIPTOR_HEAP_DESC;

        Description(DescriptorPool::Type type, int numDescriptors, Flag flags, UINT nodeMask = 0)
        {
            Super::Type           = ncast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
            Super::NumDescriptors = ncast<UINT>(numDescriptors);
            Super::Flags          = ncast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
            Super::NodeMask       = nodeMask;
        }

        D3D12_DESCRIPTOR_HEAP_DESC &operator()()
        {
            return *this;
        }
    };
    
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandleForHeapStart()
    {
        return handle->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandleForHeapStart()
    {
        return handle->GetGPUDescriptorHandleForHeapStart();
    }

    template <class T>
    T Get()
    {
        if constexpr (is_same<T, D3D12_CPU_DESCRIPTOR_HANDLE>())
        {
            return handle->GetCPUDescriptorHandleForHeapStart();
        }
        if constexpr (is_same<T, D3D12_GPU_DESCRIPTOR_HANDLE>())
        {
            return handle->GetGPUDescriptorHandleForHeapStart();
        }
    }

public:
    DescriptorPool(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_DESC *desc)
    {
        Check(device->CreateDescriptorHeap(desc, IID_PPV_ARGS(&handle)));
    }

    ~DescriptorPool()
    {
        IfNotNullThenRelease(handle);
    }

    ID3D12DescriptorHeap *Handle()
    {
        return handle;
    }

    ID3D12DescriptorHeap **AddressOf()
    {
        return &handle;
    }

private:
    ID3D12DescriptorHeap *handle{ nullptr };
};

}
}
