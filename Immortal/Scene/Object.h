#pragma once

#include "Core.h"

#include "entt.hpp"
#include "Scene.h"
#include "Component.h"

namespace Immortal
{

class IMMORTAL_API Object
{
public:
    Object() = default;

    Object(entt::entity handle, Scene *scene) :
        handle{ handle },
        scene{ scene }
    {

    }

    Object(int handle, Scene *scene) :
        handle{ entt::entity(handle) },
        scene{ scene }
    {

    }

    ~Object()
    {
    
    }

    template <class T>
    void RemoveComponent()
    {
        scene->Registry().remove<T>(handle);
    }

    template <class T, class... Args>
    T &AddComponent(Args&&... args)
    {
        if (HasComponent<T>())
        {
            LOG::WARN("Add component to a enity which alread has an identical component. The previous one would be remove.");
            RemoveComponent<T>();
        }
        return scene->Registry().emplace<T>(handle, std::forward<Args>(args)...);
    }

    template <class T>
    T &GetComponent() const
    {
        return scene->Registry().get<T>(handle);
    }

    template <class T>
    bool HasComponent()
    {
        return scene->Registry().has<T>(handle);
    }

    template <class T>
    void Remove()
    {
        RemoveComponent<T>();
    }

    template <class T, class... Args>
    T &Add(Args&&... args)
    {
        return AddComponent<T>(std::forward<Args>(args)...);
    }

    template <class T>
    T &Get() const
    {
        return GetComponent<T>();
    }

    template <class T>
    bool Has()
    {
        return HasComponent<T>();
    }

    TransformComponent &Transform() 
    {
        return GetComponent<TransformComponent>();
    }

    const Vector::mat4 &Transform() const
    {
        return GetComponent<TransformComponent>().Transform();
    }

    operator uint64_t() const
    {
        return (uint64_t)handle;
    }

    operator entt::entity() const
    {
        return handle;
    }

    operator bool() const
    {
        return uint64_t(handle) && scene;
    }

    bool operator==(const Object &other) const
    {
        return handle == other.handle && scene == other.scene;
    }

    bool operator!=(const Object &other) const
    {
        return !(*this == other);
    }

    Object &operator=(const Object &o)
    {
        THROWIF(&o == this, SError::SelfAssignment);

        scene  = o.scene;
        handle = o.handle;

        return *this;
    }

private:
    Scene *scene{ nullptr };

    entt::entity handle{ entt::null };
};

}
