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

    Anonymous address = malloc(size);

    AllocateInfo info{};
    info.clock = clock();
    info.size  = size;
    info.name  = (const char *)address;

    {
        std::lock_guard lock{ mutex };
        allocation->insert({ (uint64_t)address, info });
    }
    allocatedSize += size;

    return address;
}

void MemoryAllocator::Free(Anonymous _ptr)
{
    uint64_t ptr = (uint64_t)_ptr;

    if (allocation)
    {
        std::lock_guard lock{ mutex };
        if (allocation->find(ptr) != allocation->end())
        {
            allocation->erase((uint64_t)ptr);
        }
    }
    free(_ptr);
}

void MemoryAllocator::Release()
{
    if (!allocation)
    {
        return;
    }
    printf("Total Size Allocated: %zd (Bytes), %g (Mb)\n", allocatedSize, allocatedSize / 1048576.0);
    size_t leakSize = 0;

    for (auto &alloc : *allocation)
    {
        auto &address = std::get<0>(alloc);
        auto &allocateInfo = std::get<1>(alloc);
        leakSize += allocateInfo.size;
    }
    printf("Total Size Leaked: %d\n", (int)leakSize);

    allocation = nullptr;
}

}
