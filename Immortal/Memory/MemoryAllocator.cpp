/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "MemoryAllocator.h"

namespace Immortal
{

MemoryAllocator MemoryAllocator::Instance;

MemoryAllocator::MemoryAllocator()
{
    allocation = new Allocation;
}

MemoryAllocator::~MemoryAllocator()
{
    Release();
}

void *MemoryAllocator::Allocate(size_t size)
{
    if (!allocation)
    {
        return malloc(size);
    }

    Anonymous address = malloc(size);

    AllocationInfo info{};
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
        auto &AllocationInfo = std::get<1>(alloc);
        leakSize += AllocationInfo.size;
    }
    printf("Total Size Leaked: %d\n", (int)leakSize);

    allocation = nullptr;
}

}

extern "C"
{

void *iml_allocate(size_t size)
{
    auto &allocator = Immortal::MemoryAllocator::Instance;
    return allocator.Allocate(size);
}

void iml_release(void *ptr) noexcept
{
    auto &allocator = Immortal::MemoryAllocator::Instance;
    allocator.Free(ptr);
}

}
