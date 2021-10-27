#pragma once
#include <Immortal.h>
#include "Framework/Main.h"

using namespace Immortal;

static std::map<std::string, std::string> lt;

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
            lt[it.key()] = it.value().get<std::string>();
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

    bool LoadObject();

public:
    struct
    {
        bool showDemoWindow{ true };
        Vector4 clearColor{ 0.45f, 0.55f, 0.60f, 1.00f };
    } Settings;

    LanguageSettings languageSettings{ "Assets/json/default_language.json" };

    std::shared_ptr<Texture> primary;

    Widget::Viewport viewport{ "Viewport" };

    Widget::MenuBar menuBar;
};


class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Immortal Editor"), 1920, 1080 })
    {
        PushLayer(new VulkanLayer());
    }

    ~VulkanSample()
    {

    }

private:
    std::shared_ptr<Shader> shader;
};

Immortal::Application *Immortal::CreateApplication()
{
    Render::Set(Render::Type::Vulkan);
    return new VulkanSample();
}
