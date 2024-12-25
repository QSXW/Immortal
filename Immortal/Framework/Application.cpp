#include "Application.h"

#include "Log.h"
#include "Async.h"
#include "Render/Graphics.h"
#include "Script/ScriptEngine.h"
#include "Graphics/AsyncCompute.h"

namespace Immortal
{

Application *Application::This = nullptr;

Application::Application(BackendAPI graphicsBackendAPI, int deviceId, const std::string &title, uint32_t width, uint32_t height) :
    eventSink{ this },
    name{ title }
{
	!!This ? throw Exception(SError::InvalidSingleton) : This = this;

    eventSink.Listen(&Application::OnWindowClosed, Event::Type::WindowClose);
    eventSink.Listen(&Application::OnWindowResize, Event::Type::WindowResize);
    eventSink.Listen(&Application::OnWindowMove,   Event::Type::WindowMove);

    Async::Init();

	window = Window::CreateInstance(title, width, height, graphicsBackendAPI == BackendAPI::OpenGL ? WindowType::GLFW : WindowType::None);
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

void Application::OnRender()
{
    Time::DeltaTime = timer.tick<Timer::Seconds>();
	uint64_t syncValue = gpuEvent->GetSyncPoint() + 1;

    if (!runtime.minimized)
	{
		gpuEvent->Wait(syncValues[syncPoint], kMaxTimeOut);
		swapchain->PrepareNextFrame();
		Graphics::SetRenderIndex(gpuEvent, syncValue);

		CommandBuffer *commandBuffer = GetCurrentCommandBuffer();
		commandBuffer->Begin();
    }

	Graphics::Execute<AsyncTask>(AsyncTaskType::BeginRecording);
    for (Layer *layer : layerStack)
    {
        layer->OnUpdate();
    }

    if (!runtime.minimized)
	{
		gui->Begin();
		gui->Render();
		gui->End();
	}

    Graphics::Execute<AsyncTask>(AsyncTaskType::EndRecording);
	Graphics::Execute<AsyncTask>(AsyncTaskType::Submiting);

    if (!runtime.minimized)
	{
		CommandBuffer *commandBuffer = GetCurrentCommandBuffer();

        const float clearColor[4] = { 0, 0, 0, 0 };
        RenderTarget *renderTarget = swapchain->GetCurrentRenderTarget();
	    commandBuffer->BeginRenderTarget(renderTarget, clearColor);

		gui->SubmitRenderDrawCommands(commandBuffer, gpuEvent, syncValue);
	    commandBuffer->EndRenderTarget();
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

    window->ProcessEvents();
}

void Application::Run()
{
	Graphics::Execute<AsyncTask>(AsyncTaskType::EndRecording);
	Graphics::Execute<AsyncTask>(AsyncTaskType::Submiting);
	Graphics::WaitIdle();

	window->Show();
    while (runtime.running)
    {
        OnRender();
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
    auto width  = e.Width();
    auto height = e.Height();

	runtime.minimized = e.Width() == 0 || e.Height() == 0;

    if (!runtime.minimized)
    {
		queue->WaitIdle(0xffffffff);
		swapchain->Resize(width, height);
    }

    OnRender();

    return runtime.minimized;
}

bool Application::OnWindowMove(WindowMoveEvent &e)
{
    OnRender();
    return true;
}

}
