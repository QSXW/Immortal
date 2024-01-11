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
#include "Shared/IObject.h"
#include "Graphics/LightGraphics.h"

namespace Immortal
{

class RenderContext;

struct Configuration
{
    float FontSize{ 12.0f };
};

class ScriptEngine;
class IMMORTAL_API Application
{
public:
    Application(BackendAPI graphicsBackendAPI, const std::string &title, uint32_t width, uint32_t height);

    virtual ~Application();

    virtual void Run();

    virtual void Close();

    void OnEvent(Event &e);

    void OnRender();

    virtual Layer *PushLayer(Layer *layer);

    virtual Layer *PushOverlay(Layer *overlay);

    virtual GuiLayer *GetGuiLayer() const
    {
        return gui;
    }

    virtual Window *GetWindow() const
    {
        return window;
    }

    RenderContext *Context()
    {
        return nullptr;
    }

public:
    static uint32_t GetWidth()
    {
		return This->window->GetWidth();
    }

    static uint32_t Height()
    {
		return This->window->GetHeight();
    }

    static const char *Name()
    {
		return This->name.c_str();
    }

    static float DeltaTime()
    {
		return This->deltaTime;
    }

    static void SetTitle(const std::string &title)
    {
		return This->window->SetTitle(title);
    }

private:
    bool OnWindowClosed(WindowCloseEvent &e);

    bool OnWindowResize(WindowResizeEvent &e);

    bool OnWindowMove(WindowMoveEvent &e);

private:
	URef<Window> window;

    URef<Instance> instance;

    URef<Device> device;

    URef<Queue> queue;

    URef<Swapchain> swapchain;

    URef<GPUEvent> gpuEvent;

    std::vector<URef<CommandBuffer>> commandBuffers;

    uint32_t bufferCount = 3;

	uint32_t syncPoint = 0;

	uint32_t syncValues[3] = {};

    Ref<ScriptEngine> scriptEngine;

    struct
    {
        bool running   = true;
        bool minimized = false;
    } runtime;

    LayerStack layerStack;

    URef<GuiLayer> gui;

    std::string name;

    Timer timer;

    float deltaTime;

    EventSink<Application> eventSink;

public:
	static Application *This;

    Configuration configuration{};
};

}
