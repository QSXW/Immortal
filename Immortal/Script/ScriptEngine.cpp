#include "ScriptEngine.h"

#include "Framework/Input.h"
#include "Framework/Utils.h"
#include "Scene/Object.h"
#include "Scene/Scene.h"

namespace Immortal
{

#if HAVE_MONO
ScriptEngine *ScriptEngine::instance{};

namespace SceneComponentFunc
{

static void SetTag(uint64_t scene, int id, MonoString *text)
{
    Object object{ id, (Scene *)scene };
    auto &tag = object.GetComponent<TagComponent>();

    tag.Tag = mono_string_to_utf8(text);
}

static MonoString *GetTag(uint64_t scene, int id)
{
    Object object{ id, (Scene *)scene };
    auto &tag = object.GetComponent<TagComponent>();

    return mono_string_new(mono_domain_get(), tag.Tag.c_str());
}

static void GetPosition(uint64_t scene, int id, Vector3 *position)
{
    Object object{ id, (Scene *)scene };
    *position = object.GetComponent<TransformComponent>().Position;
}

static void SetPosition(uint64_t scene, int id, Vector3 *position)
{
    Object object{ id, (Scene *)scene };
    object.GetComponent<TransformComponent>().Position = *position;
}

static void GetRotation(uint64_t scene, int id, Vector3 *rotation)
{
    Object object{ id, (Scene *)scene };
    *rotation = object.GetComponent<TransformComponent>().Rotation;
}

static void SetRotation(uint64_t scene, int id, Vector3 *rotation)
{
    Object object{ id, (Scene *)scene };
    object.GetComponent<TransformComponent>().Rotation = *rotation;
}

static void GetScale(uint64_t scene, int id, Vector3 *scale)
{
    Object object{ id, (Scene *)scene };
    *scale = object.GetComponent<TransformComponent>().Scale;
}

static void SetScale(uint64_t scene, int id, Vector3 *scale)
{
    Object object{ id, (Scene *)scene };
    object.GetComponent<TransformComponent>().Scale = *scale;
}

static void GetTransform(uint64_t scene, int id, Matrix4 *transform)
{
    Object object{ id, (Scene *)scene };
    *transform = object.GetComponent<TransformComponent>();
}

static void SetTransform(uint64_t scene, int id, Matrix4 *value)
{
    Matrix4 &transform = *value;
    Object object{ id, (Scene *)scene };
    auto &c = object.GetComponent<TransformComponent>();
    c.Position = Vector4{ transform[0] };
    c.Rotation = Vector4{ transform[1] };
    c.Scale    = Vector4{ transform[2] };
}

}

namespace SceneObjectFunc
{

static std::unordered_map<MonoType *, std::function<bool(Object&)>> TypeReflectionHasComponent;
static std::unordered_map<MonoType *, std::function<void(Object&)>> TypeReflectionAddComponent;
#define RegisterComponent(T) {                                                                                           \
    static char name[] = "Immortal." #T;                                                                                 \
    MonoType *type = mono_reflection_type_from_name(name, image);                                                        \
    if (!type) { LOG::ERR("The type is not able to reflected from CShapr to Immortal Engine!");       }                  \
    SceneObjectFunc::TypeReflectionHasComponent[type] = [](Object &object) -> bool { return object.HasComponent<T>(); }; \
    SceneObjectFunc::TypeReflectionAddComponent[type] = [](Object &object) -> void {        object.AddComponent<T>(); }; \
}

static bool HasComponent(uint64_t scene, int id, Anonymous reflectionType)
{
    Object object{ id, (Scene *)scene };
    MonoType *type = mono_reflection_type_get_type((MonoReflectionType*)reflectionType);
    return TypeReflectionHasComponent[type](object);
}

static void AddComponent(uint64_t scene, int id, Anonymous reflectionType)
{
    Object object{ id, (Scene *)scene };
    MonoType* type = mono_reflection_type_get_type((MonoReflectionType*)reflectionType);
    TypeReflectionAddComponent[(MonoType *)type](object);
}

}

static void LogDebugCallback(MonoString *message)
{
	LOG::DEBUG("{}", Utils::ws2s(std::wstring{mono_string_chars(message)}));
}

static void LogErrorCallback(MonoString *message)
{
	LOG::ERR("{}", Utils::ws2s(std::wstring{mono_string_chars(message)}));
}

static void LogWarnCallback(MonoString *message)
{
	LOG::WARN("{}", Utils::ws2s(std::wstring{mono_string_chars(message)}));
}

static void LogInfoCallback(MonoString *message)
{
	LOG::INFO("{}", Utils::ws2s(std::wstring{mono_string_chars(message)}));
}

static void LogFatalCallback(MonoString *message)
{
	LOG::FATAL("{}", Utils::ws2s(std::wstring{mono_string_chars(message)}));
}

#define NS(n) "Immortal."#n
ScriptEngine::ScriptEngine(const std::string &name, const std::string &binary) :
    domain{},
    assembly{},
    image{}
{
    !!instance ? throw Exception(SError::InvalidSingleton) : instance = this;
    LOG::INFO("Script Driver: Init script with name {}", name);

    domain = mono_jit_init(name.c_str());
    if (!domain)
    {
        LOG::ERR("Failed to init mono domain: {}", binary);
        return;
    }

    LOG::INFO("Script Driver: Loading binary {}", binary);
    assembly = mono_domain_assembly_open(domain, binary.c_str());
    if (!assembly)
    {
        LOG::ERR("Failed to init mono assembly: {}", binary);
        return;
    }

    image = mono_assembly_get_image(assembly);

    Init();
}

ScriptEngine::~ScriptEngine()
{
    if (domain)
    {
        mono_jit_cleanup(domain);
    } 
}

MonoObject *ScriptEngine::Invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc)
{
    return mono_runtime_invoke(method, obj, params, exc);
}

MonoMethod *ScriptEngine::Search(const char *name)
{
    auto desc = mono_method_desc_new(name, false);
    return Search(desc);
}

Anonymous ScriptEngine::GetVirtualFunction(Anonymous object, const std::string &className, const std::string &func)
{
    Anonymous ret = nullptr;

    auto vtable = vtables.find(className);
    if (vtable == vtables.end())
    {
        LOG::ERR("Class {} not existed!", className);
        return ret;
    }

    auto method = vtable->second->find(func);
    if (method == vtable->second->end())
    {
        LOG::ERR("Failed to get {}::{}", className, func);
        return ret;
    }

    return mono_object_get_virtual_method((MonoObject *)object, (MonoMethod *)method->second);
}

void ScriptEngine::Register(const std::string &className, const std::initializer_list<std::string> &virtualFuntions)
{
    vtables.insert({ className, new VTable{} });
    
    VTable *vtable = vtables.find(className)->second;
    for (auto &v : virtualFuntions)
    {
        vtable->insert({ v, Search(className + "::" + v) });
    }
}

MonoMethod *ScriptEngine::Search(MonoMethodDesc *desc)
{
    return mono_method_desc_search_in_image(desc, image);
}

MonoObject *ScriptEngine::Invoke(const char *name, void *obj, void **params, MonoObject **exc)
{
    auto desc = mono_method_desc_new(name, false);
    auto method = Search(desc);

    return Invoke(method, obj, params, exc);
}

void ScriptEngine::Register(const std::string &name, const void *func)
{
    mono_add_internal_call(name.c_str(), func);
}

void ScriptEngine::Register(const char *name, const void *func)
{
    mono_add_internal_call(name, func);
}

void ScriptEngine::Init()
{
    Register(NS(Input::GetKeyDown   ), &Input::IsKeyPressed        );
    Register(NS(Input::GetButtonDown), &Input::IsMouseButtonPressed);

    Register(NS(Log::Debug), LogDebugCallback);
    Register(NS(Log::Warn),  LogWarnCallback );
    Register(NS(Log::Info),  LogInfoCallback );
    Register(NS(Log::Error), LogErrorCallback);
    Register(NS(Log::Fatal), LogFatalCallback);

    Register(NS(TagComponent::__Get), SceneComponentFunc::GetTag);
    Register(NS(TagComponent::__Set), SceneComponentFunc::SetTag);

    Register(NS(TransformComponent::__GetPosition ), SceneComponentFunc::GetPosition);
    Register(NS(TransformComponent::__SetPosition ), SceneComponentFunc::SetPosition);
    Register(NS(TransformComponent::__GetRotation ), SceneComponentFunc::GetRotation);
    Register(NS(TransformComponent::__SetRotation ), SceneComponentFunc::SetRotation);
    Register(NS(TransformComponent::__GetScale    ), SceneComponentFunc::GetScale   );
    Register(NS(TransformComponent::__SetScale    ), SceneComponentFunc::SetScale   );
    Register(NS(TransformComponent::__GetTransform), SceneComponentFunc::GetTransform);
    Register(NS(TransformComponent::__SetTransform), SceneComponentFunc::SetTransform);

    Register(NS(GameObject::__HasComponent), SceneObjectFunc::HasComponent);
    Register(NS(GameObject::__AddComponent), SceneObjectFunc::AddComponent);

    Register(
        "GameObject", { 
            "SetId_",
            "Update",
            "OnKeyDown",
            "OnButtonDown",
        }
    );

    RegisterComponent(TagComponent);
    RegisterComponent(TransformComponent);
}

MonoObject *ScriptEngine::CreateObject(const char *namespaceName, const char *className)
{
    MonoImage  *image   = mono_assembly_get_image(assembly);
    MonoClass  *myClass = mono_class_from_name(image, namespaceName, className);
    MonoObject *object  = mono_object_new(domain, myClass);
    mono_runtime_object_init(object);

    return object;
}

#endif // HAVE_MONO

int ScriptEngine::Execute(int argc, char **argv)
{
#if HAVE_MONO
    int retval = mono_jit_exec(domain, assembly, argc, argv);
    return retval;
#else
	LOG::ERR("{}", "Unable to execute script engine without Mono!");
    return -1;
#endif
}

}
