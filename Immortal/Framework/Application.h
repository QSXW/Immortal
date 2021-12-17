#pragma once

#include "ImmortalCore.h"

#include "Timer.h"
#include "Input.h"
#include "Window.h"
#include "LayerStack.h"

#include "ImGui/GuiLayer.h"
#include "Render/RenderContext.h"

#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{

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

    virtual Layer *PushLayer(Layer *layer);

    virtual Layer *PushOverlay(Layer *overlay);

    static Application *App()
    {
        return That;
    }

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

    static bool IsKeyPressed(KeyCode code)
    {
        return That->_M_input.InternalIsKeyPressed(code);
    }

    RenderContext *Context()
    {
        return context.get();
    }

public:
    static UINT32 Width()
    { 
        return That->desc.Width;
    }

    static UINT32 Height()
    {
        return That->desc.Height;
    }

    static const char *Name()
    { 
        return That->desc.Title.c_str();
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

    Input _M_input;

    Timer timer;

    float deltaTime;

    EventSink<Application> eventSink;

    static Application *That;

public:
    Configuration configuration{};
};

Application* CreateApplication();
}
