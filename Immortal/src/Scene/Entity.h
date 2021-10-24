#pragma once

#include "ImmortalCore.h"

#include "entt.hpp"
#include "Scene.h"
#include "Component.h"

namespace Immortal
{

class IMMORTAL_API Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene *scene)
        : handle(handle), scene(scene)
    {

    }

    Entity(int handle, Scene *scene)
        : handle(entt::entity(handle)), scene(scene)
    {

    }

    ~Entity() { }

    template <class T>
    void RemoveComponent()
    {
        scene->registry.remove<T>(handle);
    }

    template <class T, class... Args>
    T &AddComponent(Args&&... args)
    {
        if (HasComponent<T>())
        {
            LOG::WARN("Add component to a enity which alread has an identical component. The previous one would be remove.");
            RemoveComponent<T>();
        }
        return scene->registry.emplace<T>(handle, std::forward<Args>(args)...);
    }

    template <class T>
    T &GetComponent() const
    {
        return scene->registry.get<T>(handle);
    }

    template <class T>
    bool HasComponent()
    {
        return scene->registry.has<T>(handle);
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

    bool operator==(const Entity &other) const
    {
        return handle == other.handle && scene == other.scene;
    }

    bool operator!=(const Entity &other) const
    {
        return !(*this == other);
    }

private:
    entt::entity handle{ entt::null };
    Scene *scene{ nullptr };

    friend class Scene;
};

}
