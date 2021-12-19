#include "DescriptorPool.h"

#include "Device.h"

namespace Immortal
{
namespace D3D12
{

std::vector<DescriptorPool> DescriptorAllocator::descriptorPools;

std::mutex DescriptorAllocator::mutex;

DescriptorPool *DescriptorAllocator::Request(Device *device, DescriptorPool::Type type)
{
    std::lock_guard<std::mutex> lock{ mutex };

    DescriptorPool::Description desc{ type, NumDescriptorPerPool, DescriptorPool::Flag::None, 1 };
    descriptorPools.emplace_back(*device, &desc);

    return &descriptorPools.back();
}

CPUDescriptor DescriptorAllocator::Allocate(Device *device, uint32_t count)
{
    if (activeDescriptorPool == nullptr || freeDescritorCount < count)
    {
        activeDescriptorPool = Request(device, type);
        avtiveDescriptor     = CPUDescriptor{ activeDescriptorPool->CPUDescriptorHandleForHeapStart() };
        freeDescritorCount   = NumDescriptorPerPool;

        if (!descriptorSize)
        {
            descriptorSize = device->DescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE(type));
        }    
    }

    CPUDescriptor descriptor = avtiveDescriptor;
    avtiveDescriptor.Offset(count, descriptorSize);
    freeDescritorCount -= count;

    return descriptor;
}

}
}
