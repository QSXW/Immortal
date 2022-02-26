 #pragma once

#include "Core.h"
#include "Allocator.h"

#include <new>

namespace Immortal
{
 
struct AllocateInfo
{
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
    MemoryAllocator() = default;

    ~MemoryAllocator();

    void *Allocate(size_t size);

    void Free(Anonymous ptr);

public:
    static void Release();

private:
    std::unique_ptr<Allocation> allocation;

    size_t allocatedSize = 0;
};

}

inline void * __cdecl operator new(size_t size)
{
    return Immortal::MemoryAllocator::Primary.Allocate(size);
}

inline void operator delete(void *ptr)
{
    Immortal::MemoryAllocator::Primary.Free(ptr);
}
