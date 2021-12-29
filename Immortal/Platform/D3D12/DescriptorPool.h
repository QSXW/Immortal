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

    D3D12_CPU_DESCRIPTOR_HANDLE StartOfCPU()
    {
        return handle->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE StartOfGPU()
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

    DescriptorPool(const DescriptorPool &other) :
        handle{ other.handle }
    {

    }

    DescriptorPool(DescriptorPool &&other) :
        handle{ std::move(other.handle) }
    {
        other.handle = nullptr;
    }

    DescriptorPool &operator=(const DescriptorPool &other)
    {
        THROWIF(&other == this, SError::SelfAssignment);

        handle = other.handle;

        return *this;
    }

    DescriptorPool &operator=(DescriptorPool &&other)
    {
        THROWIF(&other == this, SError::SelfAssignment);

        handle = other.handle;
        other.handle = nullptr;

        return *this;
    }

    ~DescriptorPool()
    {

    }

    operator ID3D12DescriptorHeap*() const
    {
        return handle;
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

    void Init(Device *device, uint32_t count = 1);

    CPUDescriptor Allocate(Device *device, uint32_t count);

    Descriptor Allocate(Device *device);

    Descriptor Bind(Device *device, size_t pos);

    D3D12_CPU_DESCRIPTOR_HANDLE StartOfHeap()
    {
        return activeDescriptorPool->StartOfCPU();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE FreeStartOfHeap()
    {
        return avtiveDescriptor.cpu;
    }

    uint32_t CountOfDescriptor()
    {
        return NumDescriptorPerPool - freeDescritorCount;
    }

    uint32_t DescriptorSize()
    {
        return descriptorSize;
    }

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
    static std::vector<std::unique_ptr<DescriptorPool>> descriptorPools;

    static std::mutex mutex;
};

}
}
