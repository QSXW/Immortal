#include "Application.h"

#include "Log.h"
#include <cmath>
#include "Async.h"
#include "Render/Graphics.h"
#include "Script/ScriptEngine.h"
#include "Graphics/AsyncCompute.h"

namespace Immortal
{

Application *Application::This = nullptr;

static bool UiScaleUsesComputePath(BackendAPI api, float scale)
{
	return std::abs(scale - 1.0f) > 1e-5f && (api == BackendAPI::D3D12 || api == BackendAPI::Vulkan);
}

Application::Application(BackendAPI graphicsBackendAPI, int deviceId, const std::string &title, uint32_t width, uint32_t height, bool borderlessWindow) :
    eventSink{ this },
    name{ title }
{
	!!This ? throw Exception(SError::InvalidSingleton) : This = this;

    eventSink.Listen(&Application::OnWindowClosed, Event::Type::WindowClose);
    eventSink.Listen(&Application::OnWindowResize, Event::Type::WindowResize);
    eventSink.Listen(&Application::OnWindowMove,   Event::Type::WindowMove);

    Async::Init(1);

	window = Window::CreateInstance(title, width, height, graphicsBackendAPI == BackendAPI::OpenGL ? WindowType::GLFW : WindowType::None, borderlessWindow);
    window->SetIcon("Assets/Icon/Terminal.png");
    window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    instance = Instance::CreateInstance(graphicsBackendAPI, window->GetType());
	device = instance->CreateDevice(deviceId == AUTO_DEVICE_ID ? 0 :deviceId);
    queue = device->CreateQueue(QueueType::Graphics);

	Graphics::SetDevice(instance, device);
	Graphics::Execute<SetQueueTask>(queue);
	Graphics::Execute<AsyncTask>(AsyncTaskType::BeginRecording);
    Graphics::ConstructGlobalVariables();

    swapchain = device->CreateSwapchain(queue, window, Format::BGRA8, bufferCount, SwapchainMode::VerticalSync);

	RefreshUiCompositeTargets();

    commandBuffers.resize(bufferCount);
    for (size_t i = 0; i < bufferCount; i++)
    {
		commandBuffers[i] = device->CreateCommandBuffer();
    }

    gpuEvent = device->CreateGPUEvent("ApplicationGPUEvent");

    gui = new GuiLayer{ device, queue, window, swapchain };
	gui->OnAttach();
}

Application::~Application()
{
	timer.Stop();

    for (auto &layer : layerStack)
    {
		layer->OnDetach();
    }

	layerStack.clear();

    gui->OnDetach();
	gui.Reset();

    Graphics::Release();

    commandBuffers.clear();
	swapchain.Reset();
	gpuEvent.Reset();
    queue.Reset();
	window.Reset();
	device.Reset();
	instance.Reset();
	Async::Release();
}

Layer *Application::PushLayer(Layer *layer)
{
    layerStack.PushLayer(layer);
    layer->OnAttach();

    return layer;
}

Layer *Application::PushOverlay(Layer *overlay)
{
    layerStack.PushOverlay(overlay);
    overlay->OnAttach();

    return overlay;
}

CommandBuffer *Application::GetCurrentCommandBuffer() const
{
	return commandBuffers[syncPoint];
}

void Application::SetWindowFullscreen(bool value)
{
	if (window)
	{
        if (!pendingWindowFullscreen && window->IsFullscreen() == value)
        {
            return;
        }
        pendingWindowFullscreen = true;
        pendingWindowFullscreenState = value;
        windowModeTransition = true;
	}
}

bool Application::IsWindowFullscreen() const
{
	return window && (pendingWindowFullscreen ? pendingWindowFullscreenState : window->IsFullscreen());
}

void Application::SetUiRenderScale(float scale)
{
	uiRenderScale = scale > 1e-6f ? scale : 1.0f;
	if (swapchain)
	{
		RefreshUiCompositeTargets();
	}
}

bool Application::UsesInternalHiResUi() const
{
	return highResolutionRenderTarget != nullptr;
}

void Application::RebuildUiCompositeTargets(uint32_t swapWidth, uint32_t swapHeight)
{
	highResolutionRenderTarget.Reset();
	uiInternalColorRT.Reset();
	uiInternalMsaaRT.Reset();

	const uint32_t iw = std::max(1u, (uint32_t)std::lroundf((float)swapWidth * uiRenderScale));
	const uint32_t ih = std::max(1u, (uint32_t)std::lroundf((float)swapHeight * uiRenderScale));
	Format format = Format::BGRA8;

	if (sampleCount > 1)
	{
		uiInternalMsaaRT = device->CreateRenderTarget(iw, ih, &format, 1, Format::None, nullptr, sampleCount);
		uiInternalColorRT = device->CreateRenderTarget(iw, ih, &format, 1, Format::None, nullptr, 1);
	}
	else
	{
		uiInternalColorRT = device->CreateRenderTarget(iw, ih, &format, 1, Format::None, nullptr, 1);
	}

	highResolutionRenderTarget = (sampleCount > 1) ? uiInternalMsaaRT : uiInternalColorRT;

	uiPresentScale.EnsureScratch(device, swapWidth, swapHeight);
}

void Application::RefreshUiCompositeTargets()
{
	if (!device || !swapchain)
	{
		return;
	}

	RenderTarget *rt = swapchain->GetCurrentRenderTarget();
	Texture *tc = rt->GetColorAttachment(0);
	const uint32_t sw = tc->GetWidth();
	const uint32_t sh = tc->GetHeight();

	if (UiScaleUsesComputePath(device->GetBackendAPI(), uiRenderScale))
	{
		if (!uiPresentScale.IsReady() && !uiPresentScale.Build(device))
		{
			LOG::WARN("Ui render scale {} ignored: ui_present_scale shader missing or failed to build.", uiRenderScale);
			uiRenderScale = 1.0f;
		}

		if (uiPresentScale.IsReady())
		{
			MSAARenderTarget.Reset();
			RebuildUiCompositeTargets(sw, sh);
			return;
		}
	}

	highResolutionRenderTarget.Reset();
	uiInternalColorRT.Reset();
	uiInternalMsaaRT.Reset();
	if (sampleCount > 1)
	{
		Format format = Format::BGRA8;
		MSAARenderTarget = device->CreateRenderTarget(sw, sh, &format, 1, {}, nullptr, sampleCount);
	}
	else
	{
		MSAARenderTarget.Reset();
	}
}

void Application::OnRender()
{
	if (rendering)
	{
		return;
	}
	rendering = true;

    if (pendingWindowFullscreen && window)
    {
        if (queue)
        {
            queue->WaitIdle(0xffffffff);
        }
        window->SetFullscreen(pendingWindowFullscreenState);
        pendingWindowFullscreen = false;
        fullscreenTransitionNeedsClear = true;
        rendering = false;
        return;
    }

    Time::DeltaTime = timer.tick<Timer::Seconds>();
	uint64_t syncValue = gpuEvent->GetSyncPoint() + 1;

    if (!runtime.minimized)
	{
		if (pendingWindowResize)
		{
			queue->WaitIdle(0xffffffff);
			swapchain->Resize(pendingWindowResizeWidth, pendingWindowResizeHeight);
			pendingWindowResize = false;
			RefreshUiCompositeTargets();
		}
        if (fullscreenTransitionNeedsClear && !pendingWindowResize)
        {
            windowModeTransition = false;
            fullscreenTransitionNeedsClear = false;
        }

		gpuEvent->Wait(syncValues[syncPoint], kMaxTimeOut);
		swapchain->PrepareNextFrame();
		Graphics::SetRenderIndex(gpuEvent, syncValue);

		CommandBuffer *commandBuffer = GetCurrentCommandBuffer();
		commandBuffer->Begin();
    }

	Graphics::Execute<AsyncTask>(AsyncTaskType::BeginRecording);

    if (!runtime.minimized)
	{
		gui->Begin();
		gui->Render();
		gui->End();

		for (Layer *layer : layerStack)
		{
			layer->OnUpdate();
		}
	}

    Graphics::Execute<AsyncTask>(AsyncTaskType::EndRecording);
	Graphics::Execute<AsyncTask>(AsyncTaskType::Submiting);

    if (!runtime.minimized)
	{
		CommandBuffer *commandBuffer = GetCurrentCommandBuffer();

        ClearValue clearValues = {};
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();
		Texture *swapColor = renderTarget->GetColorAttachment(0);

		RenderTarget *imguiRenderTarget = highResolutionRenderTarget ? highResolutionRenderTarget.Get()
		                                                                : (MSAARenderTarget ? MSAARenderTarget.Get() : renderTarget);
		commandBuffer->BeginRenderTarget(imguiRenderTarget, &clearValues);
		commandBuffer->BeginEvent("ImGui::Render");
		gui->SubmitRenderDrawCommands(commandBuffer, gpuEvent, syncValue);
		commandBuffer->EndEvent();
		commandBuffer->EndRenderTarget();

		if (highResolutionRenderTarget)
		{
			if (sampleCount > 1 && uiInternalMsaaRT)
			{
				commandBuffer->BeginEvent("ImGui::Render::ResolveImage");
				commandBuffer->ResolveImage(uiInternalColorRT->GetColorAttachment(0), uiInternalMsaaRT->GetColorAttachment(0));
				commandBuffer->EndEvent();
			}

			commandBuffer->BeginEvent("ImGui::Render::ScaleComposite");
			uiPresentScale.Composite(commandBuffer, uiInternalColorRT->GetColorAttachment(0), swapColor, swapColor->GetWidth(), swapColor->GetHeight());
			commandBuffer->EndEvent();
		}
		else if (MSAARenderTarget)
		{
			commandBuffer->BeginEvent("ImGui::Render::ResolveImage");
			commandBuffer->ResolveImage(renderTarget->GetColorAttachment(0), MSAARenderTarget->GetColorAttachment(0));
			commandBuffer->EndEvent();
		}
	    commandBuffer->End();

		queue->Submit(commandBuffer, gpuEvent, swapchain);
		queue->Present(swapchain, nullptr, 0);

	    syncValues[syncPoint] = syncValue;
	    SLROTATE(syncPoint, bufferCount);
    }
    else
    {
		std::this_thread::sleep_for(std::chrono::duration(std::chrono::microseconds(16669)));
    }

	rendering = false;
}

void Application::Run()
{
	Graphics::Execute<AsyncTask>(AsyncTaskType::EndRecording);
	Graphics::Execute<AsyncTask>(AsyncTaskType::Submiting);
	Graphics::WaitIdle();

	windowShown = false;
	window->Show();
	windowShown = true;

    while (runtime.running)
    {
        OnRender();
		window->ProcessEvents();
    }
}

void Application::Close()
{
    runtime.running = false;
}

void Application::OnEvent(Event &e)
{
    eventSink.Dispatch(e);
    if (gui)
    {
		gui->OnEvent(e);
    }

    for (auto it = layerStack.end(); it != layerStack.begin(); )
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
        {
            break;
        }
    }
}

bool Application::OnWindowClosed(WindowCloseEvent &e)
{
    /* Wait all threads to finish before closing */

    runtime.running = false;
    return !runtime.running;
}

bool Application::OnWindowResize(WindowResizeEvent &e)
{
    if (!swapchain)
    {
		return false;
    }

	auto width  = e.Width();
    auto height = e.Height();

	runtime.minimized = e.Width() == 0 || e.Height() == 0;

	if (runtime.minimized)
	{
		pendingWindowResize = false;
		return runtime.minimized;
	}

	if (rendering || windowModeTransition)
	{
		pendingWindowResize = true;
		pendingWindowResizeWidth = width;
		pendingWindowResizeHeight = height;
		return false;
	}

    if (!runtime.minimized)
    {
		queue->WaitIdle(0xffffffff);
		swapchain->Resize(width, height);
		RefreshUiCompositeTargets();
    }

	if (windowShown && !rendering && !windowModeTransition)
	{
		OnRender();
	}

    return runtime.minimized;
}

bool Application::OnWindowMove(WindowMoveEvent &e)
{
	if (!swapchain)
	{
		return false;
    }

	if (windowShown && !rendering && !windowModeTransition && !pendingWindowResize)
	{
		OnRender();
	}

    return true;
}

}
