/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <queue>

#include "Core.h"
#include "Shared/Async.h"
#include "Graphics/LightGraphics.h"
#include "Render/Graphics.h"
#include "Render/Mesh.h"
#include "Vision/Picture.h"
#include "SceneCamera.h"
#include "Codec.h"
#include "Demuxer.h"
#include <map>

namespace Immortal
{

class Scene;
struct Component
{
    enum class Type
    {
        None,
        Camera,
        ColorMixing,
        DirectionalLight,
        Filter,
        ID,
        Light,
        Mesh,
        Meta,
        Material,
        NativeScript,
        Script,
        Scene,
        SpriteRenderer,
        Tag,
        Transform,
        VideoPlayer
    };

    Component()
    {

    }
};

#define DEFINE_COMPONENT_TYPE(T) static Component::Type GetType() { return Component::Type::T; }

struct IDComponent : public Component
{
    DEFINE_COMPONENT_TYPE(ID)

    IDComponent(uint64_t id = 0) :
        uid(id)
    {

    }

    uint64_t uid;
};

struct TagComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Tag)

    TagComponent(const std::string &tag = "") :
        Tag{ tag }
    {

    }

    std::string Tag;
};

struct TransformComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Transform)

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

    static inline Vector3 Up{ 0.0f, 1.0f, 0.0f };

    static inline Vector3 Right{ 1.0f, 0.0f, 0.0f };

    static inline Vector3 Forward{ 0.0f, 0.0f, -1.0f };

    Vector3 Position{ 0.0f, 0.0f, 0.0f };

    Vector3 Rotation{ 0.0f, 0.0f, 0.0f };

    Vector3 Scale{ 1.0f, 1.0f, 1.0f };
};

struct MeshComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Mesh)

    MeshComponent()
    {

    }

    MeshComponent(std::shared_ptr<Immortal::Mesh> mesh) :
        Mesh{ mesh }
    {

    }

    operator std::shared_ptr<Immortal::Mesh>()
    {
        return Mesh;
    }

    MeshComponent &operator=(const MeshComponent &other)
    {
        Mesh = other.Mesh;
        return *this;
    }

    std::shared_ptr<Immortal::Mesh> Mesh;
};

struct MaterialComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Material)

    MaterialComponent()
    {
        References.resize(1);
    }

    struct Reference
    {
        Reference() :
            AlbedoColor{ 0.995f, 0.995f, 0.995f, 1.0f },
            Metallic{ 1.0f },
            Roughness{ 1.0f }
        {
            Textures.Albedo = Graphics::Preset()->Textures.White;
			Textures.Normal = Graphics::Preset()->Textures.Normal;
            Textures.Metallic = Textures.Albedo;
            Textures.Roughness = Textures.Albedo;
            Textures.AO = Textures.Albedo;
        }

        struct {
            Ref<Texture> Albedo;
            Ref<Texture> Normal;
            Ref<Texture> Metallic;
            Ref<Texture> Roughness;
            Ref<Texture> AO;
        } Textures;

        Vector4 AlbedoColor;
        float   Metallic;
        float   Roughness;
    };

    std::vector<Reference> References;
};

struct LightComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Light)

    Vector4 Radiance{ 1.0f };
    bool Enabled = true;
};

struct SceneComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Scene)
};

struct SpriteRendererComponent : public Component
{
    DEFINE_COMPONENT_TYPE(SpriteRenderer)

    SpriteRendererComponent();

    ~SpriteRendererComponent();

    Ref<Texture> Sprite;

    Ref<Texture> Result = Sprite;

	Vector4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

	float TilingFactor = 1.0f;
};

struct CameraComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Camera)

    CameraComponent()
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
    DEFINE_COMPONENT_TYPE(DirectionalLight)

    Vector3 Radiance{ 1.0f, 1.0f, 1.0f };

    float Intensity   = 1.0f;
    bool  CastShadows = true;
    bool  SoftShadows = true;
    float LightSize   = 0.5f; // For PCSS
};

struct ScriptComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Script)

    ScriptComponent() :
        path{},
        vtable{},
        object{}
    {

    }

    ScriptComponent(const ScriptComponent &other) = default;

    ScriptComponent(const std::string &path) :
        vtable{},
        object{},
        path{ path }
    {

    }

    void Init(int id, Scene *scene);

    void Update(int id, Scene *scene, float deltaTime);

    void OnKeyDown(int id, Scene *scene, int keyCode);

    struct {
        Anonymous __setId;
        Anonymous __update;
        Anonymous __onKeyDown;
    } vtable;

    Anonymous object;

    std::string path;
    std::string className;
};

struct MetaComponent : public Component
{
    DEFINE_COMPONENT_TYPE(Meta)

    void *Meta = nullptr;
};

struct ColorMixingComponent : public Component
{
    DEFINE_COMPONENT_TYPE(ColorMixing)

    bool Modified    = false;
    bool Initialized = false;

    Vector4 RGBA = Vector4{ 0, 0, 0, 1.0f };
    Vector4 HSL;

    struct {
        float ColorTemperature;
        float Hue;
    } WhiteBalance{ 0, 0 };

    struct {
        float White;
        float Black;
    } Gradation{ 0, 0 };

    float Exposure    = 0;
    float Contrast    = 0;
    float Hightlights = 0;
    float Shadow      = 0;
    float Vividness   = 0;

    static size_t Length;
};

inline size_t ColorMixingComponent::Length = sizeof(ColorMixingComponent) - offsetof(ColorMixingComponent, RGBA);

}
