#pragma once

#include "Core.h"
#include "Math/Vector.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"
#include "Render/Texture.h"
#include "Render/Environment.h"
#include "Render/Mesh.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Component.h"

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

class Object;
class IMMORTAL_API Scene
{
public:
    struct Limit
    {
        static constexpr int MaxLightNumber = 4;
    };

public:
    Scene(const std::string &debugName="Untitled", bool isEditorScene = false);

    ~Scene();

    void OnUpdate();

    void OnEvent();

    void OnRenderRuntime();

    void OnRenderEditor(const Camera &editorCamera);

    void OnRender(const Camera &camera);

    Object CreateObject(const std::string &name = "");

    void DestroyObject(Object &object);

    void RenderObject(std::shared_ptr<Pipeline::Graphics> pipeline, entt::entity object, TransformComponent &transform, MeshComponent &mesh, MaterialComponent &material);

    void SetViewportSize(const Vector2 &size);

    void Select(Object *object);

    Object PrimaryCameraObject();

    void Serialize(const std::string &path);

    bool Deserialize(const std::string &path);

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

    std::shared_ptr<RenderTarget> Target() const
    {
        return renderTarget;
    }

private:
    std::string debugName;

    entt::registry registry;

    entt::entity entity;

    struct {
        std::shared_ptr<Mesh> skybox;
    } meshes;

    struct {
        std::shared_ptr<TextureCube> skybox;
    } textures;

    struct {
        std::shared_ptr<GraphicsPipeline> tonemap;
        std::shared_ptr<GraphicsPipeline> pbr;
        std::shared_ptr<GraphicsPipeline> basic;
        std::shared_ptr<GraphicsPipeline> outline;
        std::shared_ptr<ComputePipeline>  colorMixing;
    } pipelines;

    struct {
        std::unique_ptr<Buffer> host;
        std::unique_ptr<Buffer> transform;
        std::unique_ptr<Buffer> shading;
    } uniforms;
    
    struct {
        std::unique_ptr<Environment> global;
    } environments;

    std::shared_ptr<RenderTarget> renderTarget;

    Vector2 viewportSize{ 0.0f, 0.0f };

    Object *selectedObject{ nullptr };

private:
    ObserverCamera observerCamera;
};

}
