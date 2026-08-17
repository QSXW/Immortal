#include "DescriptorPool.h"
#include "Memory/MemoryResource.h"

namespace Immortal
{
namespace D3D12
{

DescriptorPool::DescriptorPool() :
    NonDispatchableHandle{},
	type{},
	flags{},
    activeDescriptorHeap{},
	avtiveDescriptor{}
{

}

DescriptorPool::DescriptorPool(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
    DescriptorPool{ device, NumDescriptorPerPool * descriptorCount, type, flags }
{
	descriptorCountPerHeap  = 256 / descriptorCount;
	descriptorCountPerHeap  = std::min(descriptorCountPerHeap, uint32_t(64));
	fullMask = ((1ll << (descriptorCountPerHeap - 1)) << 1) - 1;

	descriptorCountPerHeap *= descriptorCount;
}

DescriptorPool::DescriptorPool(Device *device, uint32_t descriptorCountPerHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
	NonDispatchableHandle{ device },
	descriptorCountPerHeap{ descriptorCountPerHeap },
    type{ type },
    flags{ flags },
    activeDescriptorHeap{},
    avtiveDescriptor{},
    fullMask{}
{

}

DescriptorPool::~DescriptorPool()
{
	descriptorHeaps.clear();
	device = nullptr;
}

DescriptorHeap *DescriptorPool::AllocateNextDescriptorHeap()
{
	std::lock_guard<std::mutex> lock{mutex};
	descriptorHeaps.emplace_back(new DescriptorHeap{device, descriptorCountPerHeap, type, flags});

	return descriptorHeaps.back();
}

Descriptor DescriptorPool::AllocateWithMask(DescriptorHeap **ppHeap, uint32_t descriptorCount)
{
	uint64_t mask  = 0;
	uint64_t cmask = (1ll << 1/*descriptorCount */) - 1ll;
	uint64_t index = 0;
	for (auto &[descriptorHeap, _mask] : masks)
	{
		if (_mask)
		{
			index = CountTrailingZeros64(_mask);
			bool available = ((_mask >> index) & cmask) == cmask;
			if (available)
			{
				mask = _mask;
				*ppHeap = descriptorHeap;
				break;
			}
		}
	}

	if (!mask)
	{
		index = 0;
		mask  = fullMask;
		activeDescriptorHeap        = AllocateNextDescriptorHeap();
		masks[activeDescriptorHeap] = mask;
		if (flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			avtiveDescriptor.shaderVisibleDescriptor = activeDescriptorHeap->GetGPUDescriptorHandle();
		}
		*ppHeap = activeDescriptorHeap;
	}

	mask ^= cmask << index;
	masks[*ppHeap] = mask;

	Descriptor descriptor = (*ppHeap)->GetCPUDescriptorHandle();
	descriptor.Offset(index, descriptorCount * GetIncrementSize());

	return descriptor;
}

void DescriptorPool::Free(DescriptorHeap *descriptorHeap, Descriptor descriptor, uint32_t descriptorCount)
{
	Descriptor base = descriptorHeap->GetCPUDescriptorHandle();
	auto index = (descriptor.ptr - base.ptr) / (descriptorCount * base.GetIncrementSize());
	auto &mask = masks[descriptorHeap];
	mask |= ((1ll << 1) - 1ll) << index;
}

}
}
