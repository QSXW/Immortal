#pragma once

#include "Core.h"
#include "Interface/IObject.h"
#include "Config.h"

#include <string>
#include <unordered_map>

#if HAVE_MONO
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#endif

namespace Immortal
{

class VTable : public std::unordered_map<std::string, Anonymous>, public IObject
{

};

class IMMORTAL_API ScriptEngine : public IObject
{
public:
#if !HAVE_MONO
    ScriptEngine(const std::string &name, const std::string &binary)
    {
        LOG::ERR("There is no Mono distribution detected!");
    }
#else
    ScriptEngine() = default;

    ScriptEngine(const std::string &name, const std::string &binary);

    ~ScriptEngine();

    void Init();

    void Register(const std::string &name, const void *func);

    void Register(const char *name, const void *func);

    void Register(const std::string &className, const std::initializer_list<std::string> &virtualFuntions);

    MonoObject *Invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc);

    MonoObject *Invoke(const char *name, void *obj, void **params, MonoObject **exc);

    MonoMethod *Search(const char *name);

    MonoMethod *Search(MonoMethodDesc *desc);

    Anonymous GetVirtualFunction(Anonymous object, const std::string &className, const std::string &func);

    MonoObject *CreateObject(const char *namespaceName, const char *className);

public:
    static inline Anonymous CreateObject(const std::string &namespaceName, const std::string &className)
    {
        return instance->CreateObject(namespaceName.c_str(), className.c_str());
    }

    static inline Anonymous Search(const std::string &name)
    {
        return instance->Search(name.c_str());
    }

    static inline Anonymous Search(Anonymous object, const std::string &className, const std::string &func)
    {
        return instance->GetVirtualFunction(object, className, func);
    }

    static inline Anonymous Invoke(Anonymous method, Anonymous obj, void **params)
    {
        return instance->Invoke((MonoMethod *)method, obj, params, nullptr);
    }

public:
    MonoString *NewString(const char *text)
    {
        mono_string_new(domain, text);
    }

private:
    MonoDomain *domain;
    MonoAssembly *assembly;
    MonoImage *image;

    std::unordered_map<std::string, URef<VTable>> vtables;

private:
    static ScriptEngine *instance;
#endif

public:
    int Execute(int argc, char **argv);
};

}
