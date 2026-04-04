#pragma once

#include "Core.h"
#include "Math/Vector.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"
#include "Shared/IObject.h"
#include "String/IString.h"
#include "Graphics/LightGraphics.h"
#include "Component.h"
#include "Graphics/Event/KeyEvent.h"
#include "Render/Render2D.h"
#include "Render/FrameGraph.h"
#include "Render/MeshletTask.h"
#include "Render/DeferredTask.h"
#include <map>

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
        static constexpr int MaxGaussianKernalSize = 128;
    };

public:
    Scene(const String &name = "Untitled", bool isEditorScene = false);

    ~Scene();

    void OnUpdate();

    void OnGuiRender();

    void OnEvent();

    void OnRenderRuntime();

    void OnRenderEditor(const Camera &editorCamera);

    void OnRender(const Camera &camera);

    void Render2DComponent(const Camera &camera, CommandBuffer *commandBuffer);

    void OnRender2D(const Camera &camera, RenderTarget *renderTarget);

    Object CreateObject(const std::string &name = "");

    void DestroyObject(Object &object);

    Object Query(const std::string &name);

    void SetViewportSize(const Vector2 &size);

    const Vector2 &GetViewportSize() const;

    void Select(Object *object);

    Object PrimaryCameraObject();

    void Serialize(const std::string &path);

    bool Deserialize(const std::string &path);

    void OnKeyPressed(KeyPressedEvent &e);

    void SetFrameGraph(const Ref<FrameGraph> &frameGraph);

	/** When enabled, mesh geometry renders to an internal G-buffer; DeferredLighting task composites over the scene RT. */
	void ConfigureDeferredPipeline(bool useDeferred, const Ref<MeshletTask> &meshlet, const Ref<DeferredTask> &deferred);

	bool IsDeferredPipelineEnabled() const
	{
		return useDeferredPipeline;
	}

	void SetDeferredPBRResolve(bool enable);

	bool IsDeferredPBRResolveEnabled() const;

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

    Ref<RenderTarget> GetRenderTarget() const
    {
        return renderTarget;
    }

	/** Object-id buffer for picking: forward path uses main RT attachment 1; deferred uses G-buffer attachment 2. */
	Ref<Texture> GetObjectIdPickTexture() const;

private:
    void Init();

    void LoadEnvironment();

    void ReloadSkyBoxCube();

    void Equirect2Cube();

protected:
    String name;

    entt::registry registry;

    std::multimap<std::string, int> objects;

    struct Settings
    {
        uint32_t environmentResolution = 2048;
        float exposure = 4.5f;
        float gamma    = 2.2f;
        int kernalSize = 3;
        float sigma = 1.5;
        bool changed   = true;
    } settings;

    Ref<RenderTarget> renderTarget;

    Vector2 viewportSize{ 0.0f, 0.0f };

    Object *selectedObject{ nullptr };

    URef<Render2D> render2d;

    Ref<FrameGraph> frameGraph;

	bool useDeferredPipeline = false;

	Ref<MeshletTask> meshletTask;

	Ref<DeferredTask> deferredTask;

	Ref<RenderTarget> gbufferTarget;

private:
    SceneCamera *primaryCamera = nullptr;

    ObserverCamera observerCamera;
};

}
