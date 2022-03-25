#include "Memory.h"

namespace Immortal
{

MemoryAllocator MemoryAllocator::Primary;

MemoryAllocator::~MemoryAllocator()
{
    Release();
}

void *MemoryAllocator::Allocate(size_t size)
{
    if (!allocation)
    {
        allocation.reset(new Allocation{});
    }

    Anonymous address = nullptr;
    AllocateInfo info{};
    info.size = size;
    allocatedSize += size;

    address = malloc(size);
    allocation->insert({ (uint64_t)address, info });

    return address;
}

void MemoryAllocator::Free(Anonymous _ptr)
{
    uint64_t ptr = (uint64_t)_ptr;

    if (allocation && allocation->find(ptr) != allocation->end())
    {
        allocation->erase((uint64_t)ptr);
    }
    free(_ptr);
}

void MemoryAllocator::Release()
{
    if (!Primary.allocation)
    {
        return;
    }
    LOG::DEBUG("Total Size Allocated: {}", Primary.allocatedSize);
    size_t leakSize = 0;

    for (auto &alloc : *Primary.allocation)
    {
        auto &address = std::get<0>(alloc);
        auto &allocateInfo = std::get<1>(alloc);
        LOG::DEBUG("Memory Leak: Address: {},\tSize: {}", (void *)address, allocateInfo.size);
        leakSize += allocateInfo.size;
    }
    LOG::DEBUG("Total Size Leaked: {}", leakSize);

    Primary.allocation = nullptr;
}

}
