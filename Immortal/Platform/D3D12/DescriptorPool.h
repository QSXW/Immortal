#pragma once
#include "Common.h"

#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class DescriptorPool
{
public:
    enum class Type
    {
        ShaderResourceView = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        Sampler            = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        RenderTargetView   = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        DepthStencilView   = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        Quantity           = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES
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
        if constexpr (IsPrimitiveOf<D3D12_CPU_DESCRIPTOR_HANDLE, T>() ||
                      IsPrimitiveOf<CPUDescriptor, T>())
        {
            return T{ handle->GetCPUDescriptorHandleForHeapStart() };
        }
        if constexpr (IsPrimitiveOf<D3D12_GPU_DESCRIPTOR_HANDLE, T>() ||
                      IsPrimitiveOf<GPUDescriptor, T>())
        {
            return T{ handle->GetGPUDescriptorHandleForHeapStart() };
        }
        if constexpr (IsPrimitiveOf<Descriptor, T>())
        {
            return Descriptor{
                Get<CPUDescriptor>(),
                Get<GPUDescriptor>()
            };
        }
    }

public:
    DescriptorPool(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_DESC *desc)
    {
        Check(device->CreateDescriptorHeap(desc, IID_PPV_ARGS(&handle)));
    }

    ~DescriptorPool()
    {

    }

    ID3D12DescriptorHeap *Handle()
    {
        return handle.Get();
    }

    ID3D12DescriptorHeap **AddressOf()
    {
        return handle.GetAddressOf();
    }

private:
    ComPtr<ID3D12DescriptorHeap> handle{ nullptr };
};

class DescriptorAllocator
{
public:
    static constexpr int NumDescriptorPerPool = 256;

public:
    DescriptorAllocator(DescriptorPool::Type type, DescriptorPool::Flag flag = DescriptorPool::Flag::None) :
        type{ type },
        flag{ flag },
        activeDescriptorPool{ nullptr },
        descriptorSize{ 0 },
        freeDescritorCount{ 0 },
        avtiveDescriptor{ D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN }
    {

    }

    static DescriptorPool *Request(Device *device, DescriptorPool::Type type, DescriptorPool::Flag flag = DescriptorPool::Flag::None);

    CPUDescriptor Allocate(Device *device, uint32_t count);

    Descriptor Allocate(Device *device);

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorPool, T>())
        {
            return activeDescriptorPool;
        }
    }

private:
    DescriptorPool *activeDescriptorPool;

    Descriptor avtiveDescriptor;

    DescriptorPool::Type type;

    DescriptorPool::Flag flag;

    uint32_t descriptorSize;

    uint32_t freeDescritorCount;

public:
    static std::vector<DescriptorPool> descriptorPools;

    static std::mutex mutex;
};

}
}
