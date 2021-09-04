#include "impch.h"
#include "Application.h"

#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Log.h"

#include "Render/Renderer.h"
#include "Input.h"

namespace Immortal
{
Application *Application::instance{ nullptr };

Application::Application(const Window::Description &descrition)
{
    !!instance ? throw Exception("Unable to create Two Application") : instance = this;
    desc = descrition;
    window  = Window::Create(desc);
    context = RenderContext::Create(RenderContext::Description{ window->GetNativeWindow() });
    window->SetEventCallback(SLBIND(Application::OnEvent));
    // window->SetVSync(false);
    // Renderer::Init();
    // mGuiLayer = GuiLayer::Create();
    // PushOverlay(mGuiLayer);
    // mTimer.Start();
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
    while (running)
    {
        timer.Lap();
        deltaTime = timer.elapsed();

        for (Layer *layer : layerStack)
        {
            layer->OnUpdate();
        }

        if (!minimized)
        {
            // mGuiLayer->Begin();
            for (Layer *layer : layerStack)
            {
                layer->OnGuiRender();
            }
            //mGuiLayer->End();
        }
        window->OnUpdate();
    }
}

void Application::Close()
{
    running = false;
}

void Application::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(SLBIND(Application::OnWindowClosed));
    dispatcher.Dispatch<WindowResizeEvent>(SLBIND(Application::OnWindowResize));

    for (auto it = layerStack.end(); it != layerStack.begin(); )
    {
        (*--it)->OnEvent(e);
        /* If the event was handled. Just break from the dispatch loop. */
        if (e.Handled)
        {
            break;
        }
    }
}

bool Application::OnWindowClosed(WindowCloseEvent& e)
{
    running = false;
    return !running;
}

bool Application::OnWindowResize(WindowResizeEvent &e)
{
    if (e.Width() == 0 || e.Height() == 0)
    {
        minimized = true;
    }
    else
    {
        minimized = false;
        Renderer::OnWindowResize(e.Width(), e.Height());
    }

    return minimized;
}
}
