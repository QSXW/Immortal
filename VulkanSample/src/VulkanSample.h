#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/GuiLayer.h"
#include "Render/Render.h"

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

public:
    struct
    {
        bool showDemoWindow{ false };
        bool showAnotherWindow{ false };
        Vector4 clearColor{ 0.45f, 0.55f, 0.60f, 1.00f };
    } Settings;
};

class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Vulkan Sample (Graphics API: Vulkan)"), 1920, 1080 })
    {
        PushLayer(new VulkanLayer());
    }

    ~VulkanSample()
    {

    }

private:
    
};

Immortal::Application* Immortal::CreateApplication()
{
    Render::Set(Render::Type::Vulkan);
    return new VulkanSample();
}
