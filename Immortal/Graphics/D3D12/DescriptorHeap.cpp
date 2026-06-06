#include "DescriptorHeap.h"

#include "Device.h"

namespace Immortal
{
namespace D3D12
{

DescriptorHeap::DescriptorHeap() :
    increment{}
{

}

DescriptorHeap::DescriptorHeap(Device * device, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
    increment{ device->GetDescriptorIncrement(D3D12_DESCRIPTOR_HEAP_TYPE(type)) }
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{
        .Type           = type,
	    .NumDescriptors = descriptorCount,
        .Flags          = flags,
        .NodeMask       = 0,
    };
    Check(device->Create(&desc, &handle));
}

DescriptorHeap::~DescriptorHeap()
{
	handle.Reset();
}

}
}
