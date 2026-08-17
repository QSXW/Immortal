#pragma once

#include "Core.h"

#include "Framework/UiPresentScale.h"

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
    Application(BackendAPI graphicsBackendAPI, int deviceId, const std::string &title, uint32_t width, uint32_t height, bool borderlessWindow = false);

    virtual ~Application();

    virtual void Run();

    virtual void Close();

    void OnEvent(Event &e);

    void OnRender();

    virtual Layer *PushLayer(Layer *layer);

    virtual Layer *PushOverlay(Layer *overlay);

    CommandBuffer *GetCurrentCommandBuffer() const;

    void SetWindowFullscreen(bool value);

    bool IsWindowFullscreen() const;

public:
    virtual GuiLayer *GetGuiLayer() const
    {
        return gui;
    }

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

    static Application &Reference()
    {
		return *This;
    }

	static Window *GetMainWindow()
	{
		return This ? This->window.Get() : nullptr;
	}

	void SetUiRenderScale(float scale);

	float GetUiRenderScale() const
	{
		return uiRenderScale;
	}

	/** Logical ImGui scale for fonts and style geometry; independent of the framebuffer render scale. */
	void SetUiLayoutScale(float scale);

	float GetUiLayoutScale() const
	{
		return uiLayoutScale;
	}

	/** True when UI is rendered off-screen at GetUiRenderScale() × window resolution and composited with compute. */
	bool UsesInternalHiResUi() const;

	/** Recreate internal UI targets and the swapchain MSAA buffer after resize or after SetUiRenderScale. */
	void RefreshUiCompositeTargets();

private:
    bool OnWindowClosed(WindowCloseEvent &e);

	bool OnWindowResize(WindowResizeEvent &e);

	bool OnWindowMove(WindowMoveEvent &e);

	void RebuildUiCompositeTargets(uint32_t swapWidth, uint32_t swapHeight);

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

    Ref<RenderTarget> MSAARenderTarget;

	/** Off-screen UI at uiRenderScale × swapchain size (Vulkan/D3D12 + compute composite). Same role as {@link MSAARenderTarget} for the swapchain path. */
	Ref<RenderTarget> highResolutionRenderTarget;

	Ref<RenderTarget> uiInternalColorRT;

	Ref<RenderTarget> uiInternalMsaaRT;

	UiPresentScale uiPresentScale;

	float uiRenderScale = 1.0f;

	float pendingUiRenderScale = 1.0f;

	bool uiRenderScalePending = false;

	float uiLayoutScale = 1.0f;

	float pendingUiLayoutScale = 1.0f;

	bool uiLayoutScalePending = false;

    uint32_t sampleCount = 1;

    enum class AntiAliasingMode
	{
		None,
		MSAA,
		SMAA
	};
	AntiAliasingMode aaMode = AntiAliasingMode::MSAA;

    bool windowShown = false;

    bool rendering = false;

    bool windowModeTransition = false;
    bool pendingWindowFullscreen = false;
    bool pendingWindowFullscreenState = false;
    bool fullscreenTransitionNeedsClear = false;

    bool pendingWindowResize = false;
    uint32_t pendingWindowResizeWidth = 0;
    uint32_t pendingWindowResizeHeight = 0;

public:
	static Application *This;

    Configuration configuration{};
};

}
