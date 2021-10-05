#include "impch.h"
#include "Application.h"

#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Log.h"

#include "Render/Render.h"
#include "Input.h"

#include "ThreadManager.h"

namespace Immortal
{
Application *Application::instance{ nullptr };

Application::Application(const Window::Description &descrition)
{
    !!instance ? throw Exception("Unable to create Two Application") : instance = this;
    
    Async::INIT();

    desc = descrition;
    
    window  = Window::Create(desc);
    window->SetEventCallback(SLBIND(Application::OnEvent));

    context = RenderContext::Create(RenderContext::Description{
        window.get(),
        desc.Width,
        desc.Height
        });

    Async::Execute([&](){
        guiLayer = GuiLayer::Create(context.get());
        timer.Start();
    });

    Render::INIT(context.get());

    Async::Join();

    window->Show();

    PushOverlay(guiLayer);
}

Application::~Application()
{
    timer.Stop();
}

void Application::PushLayer(Layer *layer)
{
    layerStack.PushLayer(layer);
    layer->OnAttach();
}

void Application::PushOverlay(Layer *overlay)
{
    layerStack.PushOverlay(overlay);
    overlay->OnAttach();
}
    
void Application::Run()
{
    while (runtime.running)
    { 
        Render::PrepareFrame();
        timer.Lap();
        deltaTime = timer.elapsed();

        for (Layer *layer : layerStack)
        {
            layer->OnUpdate();
        }

        guiLayer->Begin();
        for (Layer *layer : layerStack)
        {
            layer->OnGuiRender();
        }
        Async::Join();
        guiLayer->End();

        Render::SwapBuffers();

        window->SetTitle(desc.Title);
        window->ProcessEvents();
    }
}

void Application::Close()
{
    runtime.running = false;
}

void Application::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(SLBIND(Application::OnWindowClosed));
    dispatcher.Dispatch<WindowResizeEvent>(SLBIND(Application::OnWindowResize));

    LOG::INFO("{0}", e.Stringify());
    for (auto it = layerStack.end(); it != layerStack.begin(); )
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
        {
            break;
        }
    }
}

bool Application::OnWindowClosed(WindowCloseEvent& e)
{
    runtime.running = false;
    return !runtime.running;
}

bool Application::OnWindowResize(WindowResizeEvent &e)
{
    desc.Width  = e.Width();
    desc.Height = e.Height();
    if (e.Width() == 0 || e.Height() == 0)
    {
        runtime.minimized = true;
    }
    else
    {
        runtime.minimized = false;
        Render::OnWindowResize(e.Width(), e.Height());
    }

    return runtime.minimized;
}
}
