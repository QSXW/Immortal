#pragma once

#include "ImmortalCore.h"
#include "Timer.h"

#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"
#include "Window.h"
#include "LayerStack.h"
#include "ImGui/GuiLayer.h"
#include "Render/RenderContext.h"

#include "Framework/Input.h"

#include "Render/VertexArray.h"
#include "Render/Shader.h"
#include "Render/Buffer.h"

#include "Render/OrthographicCamera.h"

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

    virtual inline void PushLayer(Layer *layer);
    virtual inline void PushOverlay(Layer *overlay);

    static Application *App()
    {
        return instance;
    }

    virtual GuiLayer *GetGuiLayer() const
    {
        return guiLayer;
    }

    virtual Window& GetWindow() const
    {
        return *window;
    }

    void* GetNativeWindow() const
    {
        return window->GetNativeWindow();
    }

    static bool IsKeyPressed(KeyCode code)
    {
        return instance->_M_input.InternalIsKeyPressed(code);
    }

    RenderContext *Context()
    {
        return context.get();
    }

public:
    static UINT32 Width()
    { 
        return instance->desc.Width;
    }

    static UINT32 Height()
    {
        return instance->desc.Height;
    }

    static const char *Name()
    { 
        return instance->desc.Title.c_str();
    }

    static float DeltaTime()
    {
        return instance->deltaTime;
    }

    static void SetTitle(const std::string &title)
    {
        instance->desc.Title = title;
    }

private:
    bool OnWindowClosed(WindowCloseEvent& e);

    bool OnWindowResize(WindowResizeEvent &e);

private:
    Scope<Window> window;

    bool running = true;

    bool minimized = false;

    LayerStack layerStack;

    GuiLayer *guiLayer;

    Window::Description desc;

    Input _M_input;

    Unique<RenderContext> context;

    Timer timer;

    float deltaTime;

    static Application *instance;

    bool rendering = false;

    bool polling = false;
public:
    Configuration configuration{};
};

Application* CreateApplication();
}
