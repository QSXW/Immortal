#pragma once

#include "ImmortalCore.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"

#include "Render/Texture.h"
#include "Render/Environment.h"
#include "Render/Mesh.h"
#include "Render/Framebuffer.h"

namespace Immortal
{

struct Light
{
    Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };

    bool Enabled = false;
};

struct PointLight
{
    Vector::Vector3 Position{ 0.0f, 0.0f, 0.0f };
    Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
};

struct DirectionalLight
{
    Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
};

// This type of light needs the most time to calculate
struct SpotLight
{
    Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
    float Falloff{ 1.0 };
    float Theta; // the radian angle of the spotlight's inner cone
    float Phi;   // the angle for the outer cone of light.
};

struct LightEnvironment
{
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

    auto& Registry()
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

    entt::entity entity;

    entt::registry registry; // Entity Context: a container that contains our enties

    EntityMap entityMap;

    std::vector<Entity *> meshEntities;

    Vector2 viewportSize{ 0.0f, 0.0f };

    std::shared_ptr<TextureCube> skyboxTexture;

    std::shared_ptr<Environment> environment;

    std::shared_ptr<Mesh> skybox;

    LightEnvironment lightEnvironment;

    std::shared_ptr<UniformBuffer> transformUniformBuffer;

    std::shared_ptr<UniformBuffer> shadingUniformBuffer;

    std::shared_ptr<VertexArray> vertexArray;

    std::shared_ptr<VertexArray> toneMap;

    std::shared_ptr<Framebuffer> framebuffer;

    friend class Entity;
    friend class SceneHierarchyPanel;
    friend class EditorLayer;

private:
    ObserverCamera observerCamera;
};

}
