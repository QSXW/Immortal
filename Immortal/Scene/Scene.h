#pragma once

#include "Core.h"

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

class Object;
class IMMORTAL_API Scene
{
public:
    struct Limit
    {
        static constexpr int MaxLightNumber = 4;
    };

    static inline InputElementDescription StandardInputElementDescription {
        { Format::VECTOR3, "POSITION" },
        { Format::VECTOR3, "NORMAL"   },
        { Format::VECTOR3, "TAGENT"   },
        { Format::VECTOR3, "BITAGENT" },
        { Format::VECTOR2, "TEXCOORD" },
    };

public:
    Scene(const std::string &debugName="Untitled", bool isEditorScene = false);

    ~Scene();

    void OnUpdate();

    void OnEvent();

    void OnRenderRuntime();

    void OnRenderEditor(const EditorCamera &editorCamera);

    void OnRender(const Camera &camera);

    Object CreateObject(const std::string &name = "");

    void DestroyObject(Object &object);

    void SetViewportSize(const Vector2 &size);

    Object PrimaryCameraObject();

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

private:
    ObserverCamera observerCamera;
};

}
