#pragma once

#include "DescriptorHeap.h"

namespace Immortal
{
namespace D3D12
{

class DescriptorPool : public NonDispatchableHandle
{
public:
    static constexpr uint32_t NumDescriptorPerPool = 64;
	D3D_SWAPPABLE(DescriptorPool)

public:
	DescriptorPool();

	DescriptorPool(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    DescriptorPool(Device *device, uint32_t descriptorCountPerHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    ~DescriptorPool();

    DescriptorHeap *AllocateNextDescriptorHeap();

    Descriptor AllocateWithMask(DescriptorHeap **ppHeap, uint32_t descriptorCount);

    void Free(DescriptorHeap *descriptorHeap, Descriptor descriptor, uint32_t descriptorCount);

public:
    uint32_t GetIncrementSize() const
    {
        return activeDescriptorHeap->GetIncrementSize();
    }

    void Swap(DescriptorPool &other)
    {
		SLASSERT(false && "Don't swap DescriptorPool!");
    }

    DescriptorHeap *GetActiveDescriptorHeap() const
    {
		return activeDescriptorHeap;
    }

protected:
	std::mutex mutex;

    DescriptorHeap *activeDescriptorHeap;

    ShaderVisibleDescriptor avtiveDescriptor;

    uint32_t descriptorCountPerHeap;

    D3D12_DESCRIPTOR_HEAP_TYPE type;

    D3D12_DESCRIPTOR_HEAP_FLAGS flags;

    std::unordered_map<DescriptorHeap *, uint64_t> masks;

    uint64_t fullMask;

    std::vector<URef<DescriptorHeap>> descriptorHeaps;
};

}
}
