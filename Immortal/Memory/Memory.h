 #pragma once

#include "Core.h"
#include "Allocator.h"

#include <new>
#include <ctime>

namespace Immortal
{

struct AllocateInfo
{
    const char *name;
    clock_t clock;
    size_t size;
};

struct Allocator
{

};

class Allocation : public std::map<uint64_t, AllocateInfo, std::less<uint64_t>, CAllocator<std::pair<const uint64_t, AllocateInfo>>>
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
    static MemoryAllocator Primary;

public:
    MemoryAllocator();

    ~MemoryAllocator();

    void *Allocate(size_t size);

    void Free(Anonymous ptr);

public:
    void Release();

private:
    std::unique_ptr<Allocation> allocation;

    size_t allocatedSize = 0;

    std::mutex mutex;
};

}

#ifdef _DEBUG
void *operator new(size_t size);

void operator delete(void *ptr) noexcept;
#endif
