#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/GuiLayer.h"
#include "Render/Render.h"

using namespace Immortal;

static std::map<std::string, std::string> languageTable;

struct LanguageSettings
{
    LanguageSettings(const std::string &path)
    {
        Profiler p{ "Initializing language settings" };
        auto json = JSON::Parse(path);

        auto &map = json["settings"]["map"];

        for (decltype(json)::iterator it = map.begin(); it != map.end(); ++it)
        {
            auto &item = it->items();            
            languageTable[it.key()] = it.value().get<std::string>();
            int pause = 0;
        }
    }
};

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
        bool showDemoWindow{ true };
        Vector4 clearColor{ 0.45f, 0.55f, 0.60f, 1.00f };
    } Settings;

    LanguageSettings languageSettings{ "Assets/json/default_language.json" };
};


class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Immortal Editor"), 1920, 1080 })
    {
        PushLayer(new VulkanLayer());

        // shader = Render::CreateShader("./Assets/shaders/glsl/texture");
        shader = Render::CreateShader("./Assets/shaders/hlsl/texture");
    }

    ~VulkanSample()
    {

    }

private:
    std::shared_ptr<Shader> shader;
};

Immortal::Application* Immortal::CreateApplication()
{
    Render::Set(Render::Type::D3D12);
    return new VulkanSample();
}
