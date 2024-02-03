#pragma once

#include "Common.h"

namespace Immortal
{
namespace Metal
{

template <class T>
class Handle
{
public:
    using Primitive = T*;
    METAL_OPERATOR_HANDLE()
    METAL_SWAPPABLE(Handle)

    Handle() :
        handle{}
    {

    }

    Handle(T *handle) :
        handle{ handle }
    {

    }

    ~Handle()
    {
        if (handle)
        {
            handle->release();
            handle = {};
        }
    }

    void Swap(Handle &other)
    {
        std::swap(handle, other.handle);
    }
};

}
}
