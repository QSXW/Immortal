/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#ifndef MEMORY_ALLOCATOR_H_
#define MEMORY_ALLOCATOR_H_

#include "Core.h"
#include "Allocator.h"
#include "Shared/IObject.h"
#include <map>
#include <ctime>
#include <mutex>

namespace Immortal
{

struct AllocationInfo
{
    const char *name;

    clock_t clock;

    size_t size;
};

class Allocation : public std::map<uint64_t, AllocationInfo, std::less<uint64_t>, CAllocator<std::pair<const uint64_t, AllocationInfo>>>
{
public:
    Allocation() = default;

    void *operator new(size_t size)
    {
        return malloc(size);
    }

    void operator delete(void *ptr)
    {
        free(ptr);
    }
};

class MemoryAllocator
{
public:
    static MemoryAllocator Instance;

public:
    MemoryAllocator();

    ~MemoryAllocator();

    void *Allocate(size_t size);

    void Free(Anonymous _ptr);

    void Release();

private:
    URef<Allocation> allocation;

    size_t allocatedSize = 0;

    std::mutex mutex;
};

}

#endif
