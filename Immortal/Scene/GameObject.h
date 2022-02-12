#pragma once

#include "Core.h"
#include "Object.h"
#include "Interface/Delegate.h"

namespace Immortal
{

class IMMORTAL_API GameObject : public Object
{
public:
    virtual ~GameObject()
    {
    
    }

    template <class T>
    T &GetComponent()
    {
        return Object::Get<T>();
    }

    template <class T, class... Args>
    T &AddComponent(Args&&... args)
    {
        return Object::Add<T>(std::forward<Args>(args)...);
    }

    virtual void OnStart()
    {

    }

    virtual void OnDestroy()
    {

    }

    virtual void OnUpdate()
    {

    }

    GameObject &operator=(const Object &o)
    {
        *dynamic_cast<Object *>(this) = o;

        return *this;
    }
};

struct NativeScriptComponent : public Component
{
    DEFINE_COMP_TYPE(NativeScript)

    NativeScriptComponent() :
        Status{ Status::NotLoaded }
    {
        delegate = std::make_shared<Delegate<void()>>();
    }

    NativeScriptComponent(const std::string &module) :
        Module{ module },
        Status{ Status::NotLoaded }
    {
        delegate = std::make_shared<Delegate<void()>>();
    }

    void Map(Object o, GameObject *script)
    {
        *script = o;
        script->OnStart();
        delegate->Map<GameObject, &GameObject::OnUpdate>(script);
    }

    void OnRuntime()
    {
        delegate->Invoke();
    }

    ~NativeScriptComponent()
    {

    }

    enum class Status
    {
        Ready,
        NotLoaded
    };

    std::string Module;

    std::shared_ptr<Delegate<void()>> delegate;

    NativeScriptComponent::Status Status;
};

}
