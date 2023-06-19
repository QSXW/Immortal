#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

template <class T>
class Handle
{
public:
    VKCPP_SWAPPABLE(Handle)

public:
    Handle() :
        handle{VK_NULL_HANDLE}
    {

    }

    Handle(T handle) :
        handle{handle}
    {
    }

    operator T() const
    {
        return handle;
    }

    void Swap(Handle &other)
    {
        std::swap(handle, other.handle);
    }

protected:
    T handle;
};

}
}
