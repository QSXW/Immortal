#include "DescriptorPool.h"

namespace Immortal
{
namespace D3D12
{

DescriptorPool::DescriptorPool() :
    NonDispatchableHandle{},
	type{},
	flags{},
    activeDescriptorHeap{},
    freeDescritorCount{},
	avtiveDescriptor{}
{

}

DescriptorPool::DescriptorPool(Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
    DescriptorPool{ device, NumDescriptorPerPool, type, flags }
{

}

DescriptorPool::DescriptorPool(Device *device, uint32_t descriptorCountPerHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
	NonDispatchableHandle{ device },
	descriptorCountPerHeap{ descriptorCountPerHeap },
    type{ type },
    flags{ flags },
    activeDescriptorHeap{},
    freeDescritorCount{},
    avtiveDescriptor{}
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

Descriptor DescriptorPool::Allocate(uint32_t descriptorCount)
{
	DescriptorHeap *descriptorHeap = nullptr;
	ShaderVisibleDescriptor descriptor;
	Allocate(&descriptorHeap, &descriptor, descriptorCount);

	return descriptor.descriptor;
}

void DescriptorPool::Allocate(DescriptorHeap **ppHeap, ShaderVisibleDescriptor *pBaseDescriptor, uint32_t descriptorCount)
{
	if (!activeDescriptorHeap || freeDescritorCount < descriptorCount)
	{
		activeDescriptorHeap        = AllocateNextDescriptorHeap();
		avtiveDescriptor.descriptor = activeDescriptorHeap->GetCPUDescriptorHandle();
		if (flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			avtiveDescriptor.shaderVisibleDescriptor = activeDescriptorHeap->GetGPUDescriptorHandle();
		}
		freeDescritorCount = NumDescriptorPerPool;
	}
	
	*ppHeap          = activeDescriptorHeap;
	*pBaseDescriptor = avtiveDescriptor;
	avtiveDescriptor.Offset(descriptorCount);

	freeDescritorCount -= descriptorCount;
}

}
}
