#include "DescriptorPool.h"

#include "Device.h"

namespace Immortal
{
namespace D3D12
{

std::vector<std::unique_ptr<DescriptorPool>> DescriptorAllocator::descriptorPools;

std::mutex DescriptorAllocator::mutex;

DescriptorPool *DescriptorAllocator::Request(Device *device, DescriptorPool::Type type, DescriptorPool::Flag flag)
{
    std::lock_guard<std::mutex> lock{ mutex };

    DescriptorPool::Description desc{ type, NumDescriptorPerPool, flag, 1 };
    descriptorPools.emplace_back(new DescriptorPool{ *device, &desc });

    return descriptorPools.back().get();
}

void DescriptorAllocator::Init(Device * device, uint32_t count)
{
    if (activeDescriptorPool == nullptr || freeDescritorCount < count)
    {
        activeDescriptorPool = Request(device, type, flag);
        avtiveDescriptor     = activeDescriptorPool->Get<Descriptor>();
        freeDescritorCount   = NumDescriptorPerPool;

        if (!descriptorSize)
        {
            descriptorSize = device->DescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE(type));
        }
    }
}

CPUDescriptor DescriptorAllocator::Allocate(Device *device, uint32_t count)
{
    Init(device, count);

    CPUDescriptor descriptor = avtiveDescriptor.cpu;
    avtiveDescriptor.Offset(count, descriptorSize);
    freeDescritorCount -= count;

    return descriptor;
}

Descriptor DescriptorAllocator::Allocate(Device *device)
{
    Init(device, 1);

    Descriptor descriptor = avtiveDescriptor;
    avtiveDescriptor.Offset(1, descriptorSize);
    freeDescritorCount--;

    return descriptor;
}

}
}
