#include "DescriptorHeap.h"

#include "Device.h"

namespace Immortal
{
namespace D3D12
{

DescriptorHeap::DescriptorHeap(Device * device, D3D12_DESCRIPTOR_HEAP_DESC *desc) :
    increment{ device->GetDescriptorIncrement(desc->Type) }
{
    Check(device->Create(desc, &handle));
}

DescriptorHeap::DescriptorHeap(Device * device, uint32_t descriptorCount, Type type, Flag flags) :
    increment{ device->GetDescriptorIncrement(D3D12_DESCRIPTOR_HEAP_TYPE(type)) }
{
    DescriptorHeap::Description desc{ type, descriptorCount, flags, 1 };
    Check(device->Create(&desc, &handle));
}

std::vector<std::unique_ptr<DescriptorHeap>> DescriptorAllocator::descriptorHeaps;

std::mutex DescriptorAllocator::mutex;

DescriptorHeap *DescriptorAllocator::Request(Device *device, DescriptorHeap::Type type, DescriptorHeap::Flag flags)
{
    std::lock_guard<std::mutex> lock{ mutex };
    descriptorHeaps.emplace_back(new DescriptorHeap{ device, NumDescriptorPerPool, type, flags });

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
