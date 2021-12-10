#pragma once

#include "ImmortalCore.h"
#include "Framework/Application.h"
#include "Framework/Object.h"
#include "Scene/ScriptableObject.h"

namespace Immortal
{
class ScriptDriver : public Object
{
public:
    static std::string AssemblyPath;
    static void *Domain;

public:
    static void On(const std::string &assemblyPath);
    static void Off();

    static void InitMono();
};

class Compiler
{
public:
    enum class Flag : int
    {
        Failed = -1,
        Succeed,
        NotFound = 1,
        InvalidParameter
    };

public:
    static std::string Log;
    static int Status;
    static std::string Compiler::CompileOptions;
    static std::string Compiler::LinkOptions;
};

class Linker
{
public:
    enum class Flag : int
    {
        Failed = -1,
        Succeed,
        NotFound = 1,
        InvalidParameter
    };

public:
    static std::string Log;
    static int Status;
};

class NativeScriptDriver : public Object
{
public:
    using Getter = void*(__cdecl *)();

public:
    template <class SucceedCallback, class FailedCallBack>
    static void On(const std::string &workspace, const std::vector<std::string> &scripts, SucceedCallback succeedCallback, FailedCallBack failedCallback)
    {
        if (scripts.empty())
        {
            return;
        }
        ReleaseLoadedLibrary();
        auto succeed = Compile(workspace, scripts);
        if (succeed == Compiler::Flag::Succeed)
        {
            auto o = LoadObject();
            succeedCallback(o);
        }
        else
        {
            failedCallback();
        }
    }

    static std::vector<ScriptableObject *> LoadObject()
    {
        std::vector<ScriptableObject *> objects;

        _M_moudle = LoadLibraryA("ScriptsCore/bin/Runtime.dll");
        if (!_M_moudle)
        {
            LOG::WARN("Failed to load Runtime.dll");
            return objects;
        }

        Getter proc = (Getter)GetProcAddress(_M_moudle, "GetCubeController");
        objects.push_back(reinterpret_cast<ScriptableObject*>(proc()));

        proc = (Getter)GetProcAddress(_M_moudle, "GetApplication");
        auto app = proc();

        *((Application **)app) = Application::App();
        return objects;
    }

    static void Off();

    static Compiler::Flag Compile(const std::string &workspace, const std::vector<std::string> &scripts);

    static void ReleaseLoadedLibrary()
    {
        if (!_M_moudle)
        {
            return;
        }
        FreeLibrary(_M_moudle);
        _M_moudle = nullptr;
    }

private:
    static HMODULE _M_moudle;
};
}

