#pragma once

#include "Core.h"
#include "Math/Vector.h"

#include "entt.hpp"
#include "Scene.h"
#include "Component.h"

namespace Immortal
{

template <class T>
inline void CopyComponent(Object &dst, const Object &src);

class IMMORTAL_API Object
{
public:
    using Primitive = entt::entity;
    
    static inline Primitive NullRef = entt::null;

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
    bool HasComponent() const
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

    /* Copy Component */
    void CopyTo(Object &other)
    {
        ThrowIf(&other == this, SError::SelfAssignment)

        CopyComponent<TagComponent>(other, *this);
        CopyComponent<TransformComponent>(other, *this);
        CopyComponent<MeshComponent>(other, *this);
        CopyComponent<MaterialComponent>(other, *this);
        CopyComponent<SpriteRendererComponent>(other, *this);
        CopyComponent<ColorMixingComponent>(other, *this);
    }

    TransformComponent &Transform() 
    {
        return GetComponent<TransformComponent>();
    }

    const Matrix4 &Transform() const
    {
        return GetComponent<TransformComponent>().Transform();
    }

    operator uint64_t() const
    {
        return (uint64_t)handle;
    }

    operator Primitive() const
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

    Object &operator=(const Object &other)
    {
        THROWIF(&other == this, SError::SelfAssignment);

        scene  = other.scene;
        handle = other.handle;

        return *this;
    }

    Object &operator==(Object &&other)
    {
        THROWIF(&other == this, SError::SelfAssignment);

        scene  = other.scene;
        handle = other.handle;

        other.scene  = nullptr;
        other.handle = NullRef;
        return *this;
    }

private:
    Scene *scene{ nullptr };

    Primitive handle{ NullRef };
};

template <class T>
inline void CopyComponent(Object &dst, const Object &src)
{
    if (src.HasComponent<T>())
    {
        if (dst.HasComponent<T>())
        {
            T &component    = src.GetComponent<T>();
            T &dstComponent = dst.GetComponent<T>();
            dstComponent = component;
        }
        else
        {
            T &dstComponent = dst.AddComponent<T>();
            T &component = src.GetComponent<T>();
            dstComponent = component;
        }
    }
}

}
