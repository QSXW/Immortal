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
    DLLLoader(const std::string &path);

    virtual ~DLLLoader();

    virtual void *GetProcessAddress(const std::string &func_name);

public:
    bool IsAvailable() const
    {
        return !!handle;
    }

    template <class T>
    T GetFunc(const std::string & func_name)
    {
        return reinterpret_cast<T>(GetProcessAddress(func_name));
    }

protected:
    Handle handle;
};

}
