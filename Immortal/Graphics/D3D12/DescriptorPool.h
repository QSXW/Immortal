#pragma once

#include "DescriptorHeap.h"

namespace Immortal
{
namespace D3D12
{

class DescriptorPool : public NonDispatchableHandle
{
public:
    static constexpr uint32_t NumDescriptorPerPool = 256;
	D3D_SWAPPABLE(DescriptorPool)

public:
	DescriptorPool();

	DescriptorPool(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    DescriptorPool(Device *device, uint32_t descriptorCountPerHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    ~DescriptorPool();

    DescriptorHeap *AllocateNextDescriptorHeap();

    void Allocate(DescriptorHeap **ppHeap, ShaderVisibleDescriptor *pBaseDescriptor, uint32_t descriptorCount);

    Descriptor Allocate(uint32_t descriptorCount);

public:
    uint32_t CountOfDescriptor()
    {
        return NumDescriptorPerPool - freeDescritorCount;
    }

    uint32_t GetIncrementSize() const
    {
        return activeDescriptorHeap->GetIncrementSize();
    }

    void Swap(DescriptorPool &other)
    {
		SLASSERT(false && "Don't swap DescriptorPool!");
    }

protected:
	std::mutex mutex;

    DescriptorHeap *activeDescriptorHeap;

    ShaderVisibleDescriptor avtiveDescriptor;

    uint32_t descriptorCountPerHeap;

    D3D12_DESCRIPTOR_HEAP_TYPE type;

    D3D12_DESCRIPTOR_HEAP_FLAGS flags;

    uint32_t freeDescritorCount;

    std::vector<URef<DescriptorHeap>> descriptorHeaps;
};

}
}
