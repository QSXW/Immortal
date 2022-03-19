#include "DescriptorHeap.h"

#include "Device.h"

namespace Immortal
{
namespace D3D12
{

std::vector<std::unique_ptr<DescriptorHeap>> DescriptorAllocator::descriptorHeaps;

std::mutex DescriptorAllocator::mutex;

DescriptorHeap *DescriptorAllocator::Request(Device *device, DescriptorHeap::Type type, DescriptorHeap::Flag flag)
{
    std::lock_guard<std::mutex> lock{ mutex };

    DescriptorHeap::Description desc{ type, NumDescriptorPerPool, flag, 1 };
    descriptorHeaps.emplace_back(new DescriptorHeap{ *device, &desc });

    return descriptorHeaps.back().get();
}

void DescriptorAllocator::Init(Device * device, uint32_t count)
{
    if (activeDescriptorHeap == nullptr || freeDescritorCount < count)
    {
        activeDescriptorHeap = Request(device, type, flag);
        avtiveDescriptor     = activeDescriptorHeap->Get<Descriptor>();
        freeDescritorCount   = NumDescriptorPerPool;
    }
}

CPUDescriptor DescriptorAllocator::Allocate(Device *device, uint32_t count)
{
    Init(device, count);

    CPUDescriptor descriptor = avtiveDescriptor.cpu;
    avtiveDescriptor.Offset(count, Increment());
    freeDescritorCount -= count;

    return descriptor;
}

Descriptor DescriptorAllocator::Allocate(Device *device)
{
    Init(device, 1);

    Descriptor descriptor = avtiveDescriptor;
    avtiveDescriptor.Offset(1, Increment());
    freeDescritorCount--;

    return descriptor;
}

Descriptor DescriptorAllocator::Bind(Device *device, size_t pos)
{
    Init(device, 1);

    Descriptor descriptor = activeDescriptorHeap->Get<Descriptor>();
    descriptor.Offset(pos, Increment());

    return descriptor;
}

}
}
