#pragma once

#include "Core.h"
#include "Math/Vector.h"

#include <entt/entt.hpp>

#include "ObserverCamera.h"
#include "Shared/IObject.h"
#include "String/IString.h"
#include "Graphics/LightGraphics.h"
#include "Component.h"
#include "Graphics/Event/KeyEvent.h"
#include "Render/Render2D.h"
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

    virtual ~Scene();

    void OnUpdate();

    virtual void OnGuiRender();

    void OnEvent();

    virtual void OnRenderRuntime();

    void Render2DComponent(const Camera &camera, CommandBuffer *commandBuffer);

    void OnRender2D(const Camera &camera, RenderTarget *renderTarget);

    Object CreateObject(const std::string &name = "");

    void DestroyObject(Object &object);

    Object Query(const std::string &name);

    virtual void SetViewportSize(const Vector2 &size);

    const Vector2 &GetViewportSize() const;

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

    virtual Ref<RenderTarget> GetRenderTarget() const
    {
		return renderTarget;
    }

	bool IsEditorScene() const { return isEditorScene; }

    const ClearValue *GetRenderTargetClearValues() const { return renderTargetClearValues.data(); }

protected:
    void Init();

protected:
    String name;

    entt::registry registry;

    std::multimap<std::string, int> objects;

    struct Settings
    {
        uint32_t environmentResolution = 2048;
        float exposure = 4.5f;
        float gamma    = 2.2f;
        bool changed   = true;
    } settings;

    Ref<RenderTarget> renderTarget;

    std::vector<ClearValue> renderTargetClearValues = {
        { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
        { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
    };

    Vector2 viewportSize{ 0.0f, 0.0f };

    Object *selectedObject{ nullptr };

    URef<Render2D> render2d;

	bool isEditorScene = false;

    SceneCamera *primaryCamera = nullptr;

    ObserverCamera observerCamera;
};

}
