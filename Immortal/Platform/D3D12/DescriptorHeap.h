#pragma once
#include "Common.h"

#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class DescriptorHeap
{
public:
    using Primitive = ID3D12DescriptorHeap;
    D3D12_OPERATOR_HANDLE()

public:
    enum Type
    {
        ShaderResourceView = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        Sampler            = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        RenderTargetView   = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        DepthStencilView   = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        Quantity           = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES
    };

    enum Flag
    {
        None          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        ShaderVisible = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
    };

    struct Description : public D3D12_DESCRIPTOR_HEAP_DESC
    {
        using Super = D3D12_DESCRIPTOR_HEAP_DESC;

        Description(DescriptorHeap::Type type, UINT numDescriptors, Flag flags, UINT nodeMask = 0)
        {
            Super::Type           = ncast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
            Super::NumDescriptors = numDescriptors;
            Super::Flags          = ncast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
            Super::NodeMask       = nodeMask;
        }

        D3D12_DESCRIPTOR_HEAP_DESC &operator()()
        {
            return *this;
        }
    };

    CPUDescriptor StartOfCPU()
    {
        return CPUDescriptor{ handle->GetCPUDescriptorHandleForHeapStart() };
    }

    GPUDescriptor StartOfGPU()
    {
        return GPUDescriptor{ handle->GetGPUDescriptorHandleForHeapStart() };
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
    DescriptorHeap(Device *device, D3D12_DESCRIPTOR_HEAP_DESC *desc);

    DescriptorHeap(Device *device, uint32_t descriptorCount, Type type, Flag flags);

    DescriptorHeap(const DescriptorHeap &other) :
        handle{ other.handle }
    {

    }

    DescriptorHeap(DescriptorHeap &&other) :
        handle{ std::move(other.handle) }
    {
        other.handle = nullptr;
    }

    ~DescriptorHeap()
    {

    }

    ID3D12DescriptorHeap **AddressOf()
    {
        return handle.GetAddressOf();
    }

    uint32_t GetIncrement() const
    {
        return increment;
    }

protected:
    uint32_t increment = 0;
};

using DescriptorPool = DescriptorHeap;

class DescriptorAllocator
{
public:
    static constexpr int NumDescriptorPerPool = 256;

public:
    DescriptorAllocator(DescriptorHeap::Type type, DescriptorHeap::Flag flag = DescriptorHeap::Flag::None) :
        type{ type },
        flag{ flag },
        activeDescriptorHeap{ nullptr },
        freeDescritorCount{ 0 },
        avtiveDescriptor{ D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN }
    {

    }

    DescriptorHeap *Request(Device *device, DescriptorHeap::Type type, DescriptorHeap::Flag flag = DescriptorHeap::Flag::None);

    void Init(Device *device, uint32_t count = 1);

    CPUDescriptor Allocate(Device *device, uint32_t count);

    Descriptor Allocate(Device *device);

    Descriptor Bind(Device *device, size_t pos);

    D3D12_CPU_DESCRIPTOR_HANDLE StartOfHeap()
    {
        return activeDescriptorHeap->StartOfCPU();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE FreeStartOfHeap()
    {
        return avtiveDescriptor.cpu;
    }

    uint32_t CountOfDescriptor()
    {
        return NumDescriptorPerPool - freeDescritorCount;
    }

    uint32_t Increment() const
    {
        return activeDescriptorHeap->GetIncrement();
    }

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorHeap, T>())
        {
            return activeDescriptorHeap;
        }
    }

private:
    DescriptorHeap *activeDescriptorHeap;

    Descriptor avtiveDescriptor;

    DescriptorHeap::Type type;

    DescriptorHeap::Flag flag;

    uint32_t freeDescritorCount;

    std::vector<URef<DescriptorHeap>> descriptorHeaps;

    std::mutex mutex;
};

}
}
