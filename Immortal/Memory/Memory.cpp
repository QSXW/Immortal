#include "Memory.h"

namespace Immortal
{

MemoryAllocator MemoryAllocator::Primary;

MemoryAllocator::~MemoryAllocator()
{
    Release();
}

MemoryAllocator::MemoryAllocator()
{
    allocation.reset(new Allocation{});
}

void *MemoryAllocator::Allocate(size_t size)
{
    if (!allocation)
    {
        return malloc(size);
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
    if (!allocation)
    {
        return;
    }
    printf("Total Size Allocated: %zd\n", allocatedSize);
    int leakSize = 0;

    for (auto &alloc : *allocation)
    {
        auto &address = std::get<0>(alloc);
        auto &allocateInfo = std::get<1>(alloc);
        leakSize += allocateInfo.size;
    }
    printf("Total Size Leaked: %d\n", leakSize);

    allocation = nullptr;
}

}
