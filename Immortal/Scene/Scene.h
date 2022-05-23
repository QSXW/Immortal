#pragma once

#include "Core.h"
#include "Math/Vector.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"
#include "Interface/IObject.h"
#include "Render/Texture.h"
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
class IMMORTAL_API Scene : public IObject
{
public:
    struct Limit
    {
        static constexpr int MaxLightNumber = 4;
    };

public:
    Scene(const std::string &name = "Untitle");

    Scene(const std::string &name, bool isEditorScene = false);

    ~Scene();

    void OnUpdate();

    void OnGuiRender();

    void OnEvent();

    void OnRenderRuntime();

    void OnRenderEditor(const Camera &editorCamera);

    void OnRender(const Camera &camera);

    Object CreateObject(const std::string &name = "");

    void DestroyObject(Object &object);

    void RenderAnimatedObject(Ref<Pipeline::Graphics> pipeline, entt::entity object, TransformComponent &transform, MeshComponent &mesh, MaterialComponent &material);

    void RenderObject(Ref<Pipeline::Graphics> pipeline, entt::entity object, TransformComponent &transform, MeshComponent &mesh, MaterialComponent &material);

    void SetViewportSize(const Vector2 &size);

    void Select(Object *object);

    Object PrimaryCameraObject();

    void Serialize(const std::string &path);

    bool Deserialize(const std::string &path);

    void OnKeyPressed(KeyPressedEvent &e);

    auto &Registry()
    {
        return registry;
    }

    const char *GetName() const
    { 
        return name.c_str();
    }

    const ObserverCamera &GetObserver() const
    { 
        return observerCamera;
    }

    const Camera *GetCamera() const
    {
        return primaryCamera;
    }

    Ref<RenderTarget> Target() const
    {
        return renderTarget;
    }

private:
    void Init();

    void LoadEnvironment();

    void ReloadSkyBoxCube();

    void Equirect2Cube();

private:
    std::string name;

    entt::registry registry;

    struct Settings
    {
        uint32_t environmentResolution = 2048;
        float exposure = 4.5f;
        float gamma    = 2.2f;
    } settings;

    struct {
        std::shared_ptr<Mesh> skybox;
    } meshes;

    struct {
        Ref<Texture> skybox;
        Ref<TextureCube> skyboxCube;
    } textures;

    struct {
       Ref<GraphicsPipeline> tonemap;
       Ref<GraphicsPipeline> pbr;
       Ref<GraphicsPipeline> basic;
       Ref<GraphicsPipeline> outline;
       Ref<GraphicsPipeline> skybox;
       Ref<GraphicsPipeline> animatedBasic;
       Ref<ComputePipeline>  colorMixing;
       Ref<ComputePipeline>  equirect2Cube;
    } pipelines;

    struct {
        Ref<Buffer> host;
        Ref<Buffer> transform;
        Ref<Buffer> shading;
    } uniforms;

    Ref<RenderTarget> renderTarget;

    Vector2 viewportSize{ 0.0f, 0.0f };

    Object *selectedObject{ nullptr };

private:
    SceneCamera *primaryCamera = nullptr;

    ObserverCamera observerCamera;
};

}
