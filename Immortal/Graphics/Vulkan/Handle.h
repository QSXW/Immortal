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
    Handle(T handle = VK_NULL_HANDLE) :
        handle{handle}
    {
    }

    operator T() const
    {
        return handle;
    }

    bool operator!() const
	{
		return handle == VK_NULL_HANDLE;
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
