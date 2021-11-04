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
    Component(uint32_t c) : Category(c) { }

    enum Category {
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
    uint32_t Category{ None };
};

struct IDComponent : public Component
{
    IDComponent() : Component(Component::ID) { }
    IDComponent(uint64_t id) : uid(id), Component(Component::ID) { }

    uint64_t uid{ 0 };
};

struct TagComponent : public Component
{
    TagComponent() : Component(Component::Tag) { }
    TagComponent(const std::string &tag) : Tag(tag), Component(Component::Tag) { }
    std::string Tag;
};

struct TransformComponent : public Component
{
    TransformComponent() : Component(Component::Transform) { }
    void Set(Vector::Vector3 position, Vector::Vector3 rotation, Vector::Vector3 scale)
    { 
        Position = position;
        Rotation = rotation;
        Scale = scale;
    }

    Vector::mat4 Transform() const
    {
        return Vector::Translate(Position) * Vector::Rotate(Rotation) * Vector::Scale(Scale);
    }

    operator Vector::mat4() const
    {
        return Transform();
    }

    Vector::Vector3 Position{ 0.0f, 0.0f, 0.0f };
    Vector::Vector3 Rotation{ 0.0f, 0.0f, 0.0f };
    Vector::Vector3 Scale{ 1.0f, 1.0f, 1.0f };

    static constexpr Vector::Vector3 Up{ 0.0f, 1.0f, 0.0f };
    static constexpr Vector::Vector3 Right{ 1.0f, 0.0f, 0.0f };
    static constexpr Vector::Vector3 Forward{ 0.0f, 0.0f, -1.0f };
};

struct MeshComponent : public Component
{
    std::shared_ptr<Immortal::Mesh> Mesh;

    MeshComponent() : Component(Component::Mesh) { }
    MeshComponent(std::shared_ptr<Immortal::Mesh> mesh) : Mesh(mesh), Component(Component::Mesh) { }
    operator std::shared_ptr<Immortal::Mesh>() { return Mesh; }
};

struct MaterialComponent : public Component
{
    std::shared_ptr<Immortal::Texture2D> AlbedoMap;
    std::shared_ptr<Immortal::Texture2D> NormalMap;
    std::shared_ptr<Immortal::Texture2D> MetalnessMap;
    std::shared_ptr<Immortal::Texture2D> RoughnessMap;

    Vector::Vector3  AlbedoColor;
    float            Metalness;
    float            Roughness;

    MaterialComponent() : Component(Component::Mesh),
        AlbedoColor{ 0.995f, 0.995f, 0.995f }, Metalness(1.0f), Roughness(1.0f)
    {
        AlbedoMap    = Render::Preset()->WhiteTexture;
        NormalMap    = AlbedoMap;
        MetalnessMap = AlbedoMap;
        RoughnessMap = AlbedoMap;
    }
};

struct LightComponent : public Component
{
    LightComponent() : Component(Component::Light) { }
};

struct SceneComponent : public Component
{
    SceneComponent() : Component(Component::Scene) { }
};

struct SpriteRendererComponent : public Component
{
    Vector::Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::shared_ptr<Texture2D> Texture;
    float TilingFactor = 1.0f;

    SpriteRendererComponent() : Component(Component::SpriteRenderer)
    {
        constexpr uint64_t white = 0xffffffff;
        Texture = Texture2D::Create(1, 1);
        Texture->SetData((void*)(&white), 4);
    }
    SpriteRendererComponent(std::shared_ptr<Texture2D> texture) : Texture(texture), Component(Component::SpriteRenderer) { }
    SpriteRendererComponent(std::shared_ptr<Texture2D> texture, const Vector::Vector4 color) : Texture(texture), Color(color), Component(Component::SpriteRenderer) { }
    SpriteRendererComponent(const SpriteRendererComponent& other) = default;
};

struct CameraComponent : public Component
{
    CameraComponent() : Component(Component::Camera) { }
    CameraComponent(const CameraComponent &other) = default;

    SceneCamera Camera;
    bool Primary = false;

    operator SceneCamera&() { return Camera; }
    operator const SceneCamera&() const { return Camera; }
};

struct DirectionalLightComponent : public Component
{
    DirectionalLightComponent() : Component(Component::Light) { }

    Vector::Vector3 Radiance = { 1.0f, 1.0f, 1.0f };
    float Intensity   = 1.0f;
    bool  CastShadows = true;
    bool  SoftShadows = true;
    float LightSize = 0.5f; // For PCSS
};

struct ScriptComponent : public Component
{
    std::string Name;

    ScriptComponent() : Component(Component::Script) { }
    ScriptComponent(const ScriptComponent & other) = default;
    ScriptComponent(const std::string &name)
        : Component(Component::Script), Name(name)
    {

    }
};
}
