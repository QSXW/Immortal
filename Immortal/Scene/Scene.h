#pragma once

#include "ImmortalCore.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"

#include "Render/Texture.h"
#include "Render/Environment.h"
#include "Render/Mesh.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"

namespace Immortal
{

struct Resolution
{
    operator std::string()
    {
        return std::string{
            std::to_string(Width) +
            std::string{ "x" } +
            std::to_string(Height)
        };
    }

    uint32_t Width;
    uint32_t Height;
};

namespace Resolutions
{
    static Resolution UHD = { 3840, 2160 };
    static Resolution FHD = { 1920, 1080 };
    static Resolution HD  = { 1280, 720  };
}

struct Light
{
    Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    Vector3 Radiance{ 0.0f, 0.0f, 0.0f };

    bool Enabled = false;
};

struct PointLight
{
    Vector3 Position{ 0.0f, 0.0f, 0.0f };
    Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
};

struct DirectionalLight
{
    Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
};

// This type of light needs the most time to calculate
struct SpotLight
{
    Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    float Falloff{ 1.0 };
    float Theta; // the radian angle of the spotlight's inner cone
    float Phi;   // the angle for the outer cone of light.
};

struct LightEnvironment
{
    LightEnvironment()
    {
        lights[0].Direction = Vector::Normalize(Vector3{ -1.0f,  0.0f, 0.0f });
        lights[1].Direction = Vector::Normalize(Vector3{ 1.0f,  0.0f, 0.0f });
        lights[2].Direction = Vector::Normalize(Vector3{ 0.0f, -1.0f, 0.0f });

        lights[0].Radiance = Vector3{ 1.0f };;
        lights[1].Radiance = Vector3{ 1.0f };;
        lights[2].Radiance = Vector3{ 1.0f };;

        lights[0].Enabled = true;
        lights[1].Enabled = true;
        lights[2].Enabled = true;
    }

    static constexpr int LightNumbers = 3;
    float pitch = 0.0f;
    float yaw   = 0.0f;
    Light lights[LightNumbers];
};

class Entity;
class IMMORTAL_API Scene
{
public:
    Scene(const std::string &debugName="Untitled", bool isEditorScene = false);

    ~Scene();

    void OnUpdate();

    void OnEvent();

    void OnRenderRuntime();

    void OnRenderEditor(const EditorCamera &editorCamera);

    Entity CreateEntity(const std::string &name = "");

    void DestroyEntity(Entity &e);

    void SetViewportSize(const Vector::Vector2 &size);

    Entity PrimaryCameraEntity();

    auto &Registry()
    {
        return registry;
    }

    const char *Name() const
    { 
        return debugName.c_str();
    }

    const ObserverCamera &Observer() const
    { 
        return observerCamera;
    }

public:
    using EntityMap = std::unordered_map<uint64_t, Entity>;

private:
    std::string debugName;

    entt::registry registry;

    entt::entity entity;

    EntityMap entityMap;

    struct {
        std::shared_ptr<Mesh> skybox;
    } meshes;

    struct {
        std::shared_ptr<TextureCube> skybox;
    } textures;

    struct {
        std::shared_ptr<Pipeline> tonemap;
    } pipelines;

    struct {
        std::unique_ptr<Buffer> transform;
        std::unique_ptr<Buffer> shading;
    } uniforms;
    
    struct {
        std::unique_ptr<Environment> global;
        LightEnvironment light;
    } environments;

    std::shared_ptr<RenderTarget> renderTarget;

    Vector2 viewportSize{ 0.0f, 0.0f };

private:
    ObserverCamera observerCamera;
};

}
