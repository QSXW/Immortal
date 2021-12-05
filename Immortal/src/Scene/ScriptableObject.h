#pragma once

#include "ImmortalCore.h"
#include "Entity.h"
#include "Interface/Delegate.h"

namespace Immortal
{

class IMMORTAL_API ScriptableObject
{
public:
    virtual ~ScriptableObject() { }

    template <class T>
    T &GetComponent()
    {
        return entity.GetComponent<T>();
    }

    template <class T, class... Args>
    T &AddComponent(Args&&... args)
    {
        return entity.AddComponent<T>(std::forward<Args>(args)...);
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

public:
    Entity entity;
};

using GameObject = ScriptableObject;

struct NativeScriptComponent : public Component
{
    NativeScriptComponent() :
        Component{ Type::Script },
        Status{ Status::NotLoaded }
    {
        delegate = std::make_shared<Delegate<void()>>();
    }

    NativeScriptComponent(const std::string &module) :
        Component{ Type::Script },
        Module{ module },
        Status{ Status::NotLoaded }
    {
        delegate = std::make_shared<Delegate<void()>>();
    }

    void Map(Entity e, ScriptableObject *script)
    {
        script->entity = e;
        script->OnStart();
        delegate->Map<ScriptableObject, &ScriptableObject::OnUpdate>(script);
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
