#pragma once

#include "Core.h"

#include "Timer.h"
#include "Input.h"
#include "Window.h"
#include "LayerStack.h"

#include "ImGui/GuiLayer.h"

#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{

class RenderContext;

struct Configuration
{
    float FontSize{ 12.0f };
};

class IMMORTAL_API Application
{
public:
    Application(const Window::Description &desc = { "Immortal Engine", 1920, 1080 });

    virtual ~Application();

    virtual void Run();

    virtual void Close();

    void OnEvent(Event &e);

    void UpdateMeta(const Window::Description &desc);

    virtual Layer *PushLayer(Layer *layer);

    virtual Layer *PushOverlay(Layer *overlay);

    virtual GuiLayer *GetGuiLayer() const
    {
        return gui;
    }

    virtual Window &GetWindow() const
    {
        return *window;
    }

    void *GetNativeWindow() const
    {
        return window->GetNativeWindow();
    }

    RenderContext *Context()
    {
        return context.get();
    }

public:
    static Application *App()
    {
        return That;
    }

    static uint32_t Width()
    {
        return That->desc.Width;
    }

    static uint32_t Height()
    {
        return That->desc.Height;
    }

    static const char *Name()
    {
        return That->name.c_str();
    }

    static float DeltaTime()
    {
        return That->deltaTime;
    }

    static void SetTitle(const std::string &title)
    {
        That->desc.Title = title;
    }

private:
    bool OnWindowClosed(WindowCloseEvent &e);

    bool OnWindowResize(WindowResizeEvent &e);

private:
    std::unique_ptr<Window> window;

    std::unique_ptr<RenderContext> context;

    struct
    {
        bool running   = true;
        bool minimized = false;
    } runtime;

    LayerStack layerStack;

    GuiLayer *gui;

    Window::Description desc;

    std::string name;

    Timer timer;

    float deltaTime;

    EventSink<Application> eventSink;

    static Application *That;

public:
    Configuration configuration{};
};

}
