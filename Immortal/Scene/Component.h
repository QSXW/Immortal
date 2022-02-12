#pragma once

#include "Core.h"

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
        Transform
    };

    Component()
    {
        
    }
};

#define DEFINE_COMP_TYPE(T) static Component::Type GetType() { return Component::Type::##T; }

struct IDComponent : public Component
{
    DEFINE_COMP_TYPE(ID)

    IDComponent(uint64_t id = 0) :
        uid(id)
    {
        
    }

    uint64_t uid;
};

struct TagComponent : public Component
{
    DEFINE_COMP_TYPE(Tag)

    TagComponent(const std::string &tag = "") :
        Tag{ tag }
    {

    }

    std::string Tag;
};

struct TransformComponent : public Component
{
    DEFINE_COMP_TYPE(Transform)

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
    DEFINE_COMP_TYPE(Mesh)

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
        ThrowIf(&other == this, SError::SelfAssignment)

        Mesh = other.Mesh;
        return *this;
    }

    std::shared_ptr<Immortal::Mesh> Mesh;
};

struct MaterialComponent : public Component
{
    DEFINE_COMP_TYPE(Material)

    MaterialComponent() :
        AlbedoColor{ 0.995f, 0.995f, 0.995f },
        Metallic{ 1.0f },
        Roughness{ 1.0f }
    {
        Textures.Albedo    = Render::Preset()->Textures.White;
        Textures.Normal    = Textures.Albedo;
        Textures.Metallic  = Textures.Albedo;
        Textures.Roughness = Textures.Albedo;
        Textures.AO        = Textures.Albedo;
    }

    struct {
        std::shared_ptr<Texture> Albedo;
        std::shared_ptr<Texture> Normal;
        std::shared_ptr<Texture> Metallic;
        std::shared_ptr<Texture> Roughness;
        std::shared_ptr<Texture> AO;
    } Textures;

    Vector3 AlbedoColor;
    float   Metallic;
    float   Roughness;
};

struct LightComponent : public Component
{
    DEFINE_COMP_TYPE(Light)

    Vector4 Radiance{ 1.0f };
    bool Enabled = true;
};

struct SceneComponent : public Component
{
    DEFINE_COMP_TYPE(Scene)
};

struct SpriteRendererComponent : public Component
{
    DEFINE_COMP_TYPE(SpriteRenderer)

    static inline Texture::Description Desc = {
        Format::RGBA8,
        Wrap::Repeat,
        Filter::Linear
    };
    
    SpriteRendererComponent()
    {

    }

    SpriteRendererComponent(std::shared_ptr<Texture> texture) :
        Texture{ texture }
    {

    }

    SpriteRendererComponent(std::shared_ptr<Texture> texture, const Vector4 color) :
        Texture{ texture },
        Color{ color }
    {

    }

    SpriteRendererComponent(const SpriteRendererComponent &other) = default;

    std::shared_ptr<Immortal::Texture> Texture = Render::Preset()->Textures.White;

    std::shared_ptr<Immortal::Texture> Final   = Texture;

    Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

    float TilingFactor = 1.0f;
};

struct CameraComponent : public Component
{
    DEFINE_COMP_TYPE(Camera)

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
    DEFINE_COMP_TYPE(DirectionalLight)

    Vector3 Radiance{ 1.0f, 1.0f, 1.0f };

    float Intensity   = 1.0f;
    bool  CastShadows = true;
    bool  SoftShadows = true;
    float LightSize   = 0.5f; // For PCSS
};

struct ScriptComponent : public Component
{
    DEFINE_COMP_TYPE(Script)

    ScriptComponent(const ScriptComponent & other) = default;

    ScriptComponent(const std::string &name) :
        Name{ name }
    {

    }

    std::string Name;
};

struct MetaComponent : public Component
{
    DEFINE_COMP_TYPE(Meta)

    void *Meta = nullptr;
};

struct ColorMixingComponent : public Component
{
    DEFINE_COMP_TYPE(ColorMixing)

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

enum class FilterType
{
    None,
    GaussianBlur,
    AverageBlur,
    DCT
};

struct FilterComponent : public Component
{
    DEFINE_COMP_TYPE(Filter)

    FilterComponent()
    {

    }

    std::vector<FilterType> Filter;
};

}
