#pragma once

#include "Core.h"

namespace Immortal
{

class IMMORTAL_API DLLLoader
{
public:
#ifdef _WIN32
    using Handle = HMODULE;
#else
    using Handle = void*;
#endif

public:
    DLLLoader();

    DLLLoader(const std::string &path);

    virtual ~DLLLoader();

    virtual void *GetProcessAddress(const std::string &func_name);

public:
    DLLLoader(DLLLoader &&other) :
        handle{}
    {
        other.Swap(*this);
    }

    DLLLoader &operator=(DLLLoader &&other)
    {
        DLLLoader(std::move(other)).Swap(*this);
        return *this;
    }

    void Swap(DLLLoader &other)
    {
        std::swap(handle, other.handle);
    }

    bool IsAvailable() const
    {
        return !!handle;
    }

    operator bool() const
    {
        return !!handle;
    }

    template <class T = Anonymous>
    T GetFunc(const std::string & func_name)
    {
        return reinterpret_cast<T>(GetProcessAddress(func_name));
    }

    DLLLoader(const DLLLoader &other) = delete;

    DLLLoader &operator=(const DLLLoader &other) = delete;

protected:
    Handle handle;
};

}
