#pragma once
#include <Immortal.h>
#include "Framework/Main.h"

#include "NetLayer.h"
#include "RenderLayer.h"

using namespace Immortal;

class VulkanLayer : public Layer
{
public:
    VulkanLayer();
    virtual ~VulkanLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnGuiRender() override;

    bool LoadObject();

public:
    struct
    {
        bool showDemoWindow{ true };
        Vector4 clearColor{ 0.45f, 0.55f, 0.60f, 1.00f };
    } Settings;

    std::shared_ptr<Texture> primary;

    std::shared_ptr<Texture> second;

    Widget::Viewport viewport{ "Viewport" };

    Widget::Viewport preview{ "preview" };

    Widget::MenuBar menuBar;
};


class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Immortal Editor"), 1920, 1080 })
    {
        PushLayer(new VulkanLayer());
        PushLayer(new NetLayer("Debug Layer for Net"));

        renderLayer = new RenderLayer(Vector2{ 1920, 1080 }, "Debug Layer for Render");
        PushLayer(renderLayer);
    }

    ~VulkanSample()
    {

    }

public:
    static inline RenderLayer *renderLayer{ nullptr };
};

Immortal::Application *Immortal::CreateApplication()
{
    Render::Set(Render::Type::Vulkan);
    return new VulkanSample();
}
