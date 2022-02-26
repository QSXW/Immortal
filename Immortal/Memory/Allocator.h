#pragma once

#include <cstdlib>

namespace Immortal
{

template <class T>
struct CAllocator
{
    typedef T value_type;

    CAllocator() noexcept
    {
    
    }

    template<class U>
    CAllocator(const CAllocator<U>&) noexcept
    {
    
    }

    template<class U>
    bool operator==(const CAllocator<U> &other) const noexcept
    {
        return this == &other;
    }

    template<class U>
    bool operator!=(const CAllocator<U> &other) const noexcept
    {
        return this != &other;
    }

    T *allocate(const size_t n) const
    {
        return (T *)malloc(sizeof(T) * n);
    }

    void deallocate(T *const ptr, size_t) const noexcept
    {
        free(ptr);
    }
};

}
