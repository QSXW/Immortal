#pragma once

#include "Core.h"
#include "Object.h"

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
    DEFINE_COMPONENT_TYPE(NativeScript)

    NativeScriptComponent() :
        Status{ Status::NotLoaded },
	    proxy{}
    {

    }

    NativeScriptComponent(const std::string &module) :
        Module{ module },
        Status{ Status::NotLoaded },
	    proxy{}
    {

    }

    void Map(Object o, GameObject *script)
    {
        *script = o;
        script->OnStart();
		proxy = [=] {
			script->OnUpdate();
		};
    }

    void OnRuntime()
    {
		proxy();
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

    std::function<void()> proxy;

    NativeScriptComponent::Status Status;
};

}
