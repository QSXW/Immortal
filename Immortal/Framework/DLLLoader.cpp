#include "DLLLoader.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace Immortal
{

DLLLoader::DLLLoader() :
    handle{}
{
    
}

#ifdef _WIN32
DLLLoader::DLLLoader(const std::string &path) :
    handle { nullptr }
{
    handle = LoadLibraryA(path.c_str());
}

DLLLoader::~DLLLoader()
{
    if (handle)
    {
        FreeLibrary(handle);
        handle = nullptr;
    }
}

void *DLLLoader::GetProcessAddress(const std::string &func)
{
    return (void *)GetProcAddress(handle, func.c_str());
}

#else
DLLLoader::DLLLoader(const std::string &path) :
    handle{ nullptr }
{
    handle = dlopen(path.c_str(), RTLD_LAZY);
}

DLLLoader::~DLLLoader()
{
    if (handle)
    {
        dlclose(handle);
        handle = nullptr;
    }
}

void *DLLLoader::GetProcessAddress(const std::string &func)
{
    return dlsym(handle, func.c_str());
}

#endif

}
