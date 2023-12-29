#pragma once
#include "Common.h"
#include "Handle.h"
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
	D3D_SWAPPABLE(DescriptorHeap)

public:
	DescriptorHeap();

    DescriptorHeap(Device *device, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

    ~DescriptorHeap();

public:
	uint32_t GetIncrementSize() const
    {
        return increment;
    }

    Descriptor GetCPUDescriptorHandle() const
    {
		return { handle->GetCPUDescriptorHandleForHeapStart(), increment };
    }

    DescriptorHandle GetGPUDescriptorHandle() const
	{
		return handle->GetGPUDescriptorHandleForHeapStart();
	}

    void Swap(DescriptorHeap &other)
    {
		std::swap(handle,    other.handle   );
		std::swap(increment, other.increment);
    }

protected:
    uint32_t increment = 0;
};

}
}
