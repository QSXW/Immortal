#include "Application.h"

#include "Log.h"
#include "Async.h"
#include "Render/Render.h"
#include "Render/RenderContext.h"
#include "Script/ScriptEngine.h"

namespace Immortal
{

Application *Application::That{ nullptr };

Application::Application(const Window::Description &description) :
    eventSink{ this }
{
    !!That ? throw Exception(SError::InvalidSingleton) : That = this;

    eventSink.Listen(&Application::OnWindowClosed, Event::Type::WindowClose);
    eventSink.Listen(&Application::OnWindowResize, Event::Type::WindowResize);

    UpdateMeta(description);

    Async::Setup();

    window.reset(Window::Create(desc));
    window->SetIcon("Assets/Icon/terminal.png");
    window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    context = RenderContext::Create(RenderContext::Description{
        window.get(),
        desc.Width,
        desc.Height
        });

    scriptEngine = new ScriptEngine{ description.Title, R"(C:\Users\qsxw\source\repos\ConsoleApp2\ConsoleApp2\bin\Debug\net6.0\ConsoleApp2.dll)" };

    Async::Execute([&](){
        gui = context->CreateGuiLayer();
        timer.Start();
    });

    Render::Setup(context.get());

    Async::Wait();

    window->Show();

    PushOverlay(gui);
}

Application::~Application()
{
    Render::Release();
    timer.Stop();
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

void Application::Run()
{
    while (runtime.running)
    {
        Render::PrepareFrame();
        Time::DeltaTime = timer.tick<Timer::Seconds>();
        
        for (Layer *layer : layerStack)
        {
            layer->OnUpdate();
        }
        gui->Begin();
        for (Layer *layer : layerStack)
        {
            layer->OnGuiRender();
        }
        gui->End();     

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
    eventSink.Dispatch(e);
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
    Async::Release();

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

void Application::UpdateMeta(const Window::Description &description)
{
    desc = description;
    name = desc.Title;
}

}
