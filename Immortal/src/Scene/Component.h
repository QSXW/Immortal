#pragma once

#include "ImmortalCore.h"

#include "Render/Render.h"
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include "SceneCamera.h"

namespace IMMORTAL_API Immortal
{

struct Component
{
    enum class Type
    {
        None,
        ID,
        Tag,
        Transform,
        Mesh,
        Light,
        Script,
        Scene,
        SpriteRenderer,
        Camera
    };

    Component(Type c) :
        type{ type }
    {
        
    }

    Type type{ None };
};

struct IDComponent : public Component
{
    IDComponent() :
        Component{ Type::ID }
    {
        
    }

    IDComponent(uint64_t id) :
        Component{ Type::ID },
        uid(id)
    {
    
    }

    uint64_t uid{ 0 };
};

struct TagComponent : public Component
{
    TagComponent() :
        Component{ Type::Tag }
    {
        
    }

    TagComponent(const std::string &tag) :
        Component{ Type::Tag },
        Tag{ tag }
    {

    }

    std::string Tag;
};

struct TransformComponent : public Component
{
    TransformComponent() :
        Component{ Type::Transform }
    {
        
    }

    void Set(Vector3 position, Vector3 rotation, Vector3 scale)
    { 
        Position = position;
        Rotation = rotation;
        Scale    = scale;
    }

    Matrix4 Transform() const
    {
        return Vector::Translate(Position) * Vector::Rotate(Rotation) * Vector::Scale(Scale);
    }

    operator Matrix4() const
    {
        return Transform();
    }

    static constexpr Vector3 Up{ 0.0f, 1.0f, 0.0f };

    static constexpr Vector3 Right{ 1.0f, 0.0f, 0.0f };

    static constexpr Vector3 Forward{ 0.0f, 0.0f, -1.0f };

    Vector3 Position{ 0.0f, 0.0f, 0.0f };

    Vector3 Rotation{ 0.0f, 0.0f, 0.0f };

    Vector3 Scale{ 1.0f, 1.0f, 1.0f };
};

struct MeshComponent : public Component
{
    MeshComponent() :
        Component{ Type::Mesh }
    {
    
    }

    MeshComponent(std::shared_ptr<Immortal::Mesh> mesh) :
        Component{ Type::Mesh },
        Mesh{ mesh }
    {
    
    }

    operator std::shared_ptr<Immortal::Mesh>() { return Mesh; }

    std::shared_ptr<Immortal::Mesh> Mesh;
};

struct MaterialComponent : public Component
{
    Vector3  AlbedoColor;
    float    Metalness;
    float    Roughness;

    MaterialComponent() :
        Component{ Type::Mesh },
        AlbedoColor{ 0.995f, 0.995f, 0.995f },
        Metalness(1.0f),
        Roughness(1.0f)
    {
        AlbedoMap    = Render::Preset()->WhiteTexture;
        NormalMap    = AlbedoMap;
        MetalnessMap = AlbedoMap;
        RoughnessMap = AlbedoMap;
    }

    std::shared_ptr<Immortal::Texture> AlbedoMap;
    std::shared_ptr<Immortal::Texture> NormalMap;
    std::shared_ptr<Immortal::Texture> MetalnessMap;
    std::shared_ptr<Immortal::Texture> RoughnessMap;
};

struct LightComponent : public Component
{
    LightComponent() :
        Component{ Type::Light }
    {
        
    }
};

struct SceneComponent : public Component
{
    SceneComponent() :
        Component{ Type::Scene }
    {
    
    }
};

struct SpriteRendererComponent : public Component
{
    SpriteRendererComponent() :
        Component{ Type::SpriteRenderer }
    {
        Texture = Render::Preset()->WhiteTexture;
    }

    SpriteRendererComponent(std::shared_ptr<Texture2D> texture) :
        Component{ Type::SpriteRenderer },
        Texture{ texture }
    {

    }

    SpriteRendererComponent(std::shared_ptr<Texture2D> texture, const Vector4 color) :
        Component{ Type::SpriteRenderer },
        Texture{ texture },
        Color{ color }
    {
    
    }

    SpriteRendererComponent(const SpriteRendererComponent &other) = default;

    std::shared_ptr<Texture> Texture;

    Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    float TilingFactor = 1.0f;
};

struct CameraComponent : public Component
{
    CameraComponent() :
        Component{ Type::Camera }
    {
    
    }

    CameraComponent(const CameraComponent &other) = default;

    operator SceneCamera&()
    { 
        return Camera;
    }

    operator const SceneCamera&() const
    { 
        return Camera;
    }

    SceneCamera Camera;

    bool Primary = false;
};

struct DirectionalLightComponent : public Component
{
    DirectionalLightComponent() :
        Component{ Type::Light }
    {
    
    }

    Vector3 Radiance{ 1.0f, 1.0f, 1.0f };

    float Intensity   = 1.0f;
    bool  CastShadows = true;
    bool  SoftShadows = true;
    float LightSize   = 0.5f; // For PCSS
};

struct ScriptComponent : public Component
{
    ScriptComponent() :
        Component{ Type::Script }
    {
    
    }

    ScriptComponent(const ScriptComponent & other) = default;

    ScriptComponent(const std::string &name) :
        Component{ Type::Script },
        Name{ name }
    {

    }

    std::string Name;
};

}
