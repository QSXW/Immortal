#include "impch.h"
#include "Application.h"

#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Log.h"

#include "Render/Renderer.h"
#include "Input.h"

namespace Immortal {

#define BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)

    Application *Application::Instance = nullptr;

    Application::Application(const Description &desc)
    {
        if (Instance)
        {
            return;
        }
        else
        {
            Instance = this;
        }
        this->desc = desc;
        window  = Window::Create({ desc.Name, desc.Width, desc.Height });
        context = RenderContext::Create(RenderContext::Description{ window->GetNativeWindow() });
        window->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));
        // window->SetVSync(false);
        // Renderer::Init();
        // mGuiLayer = GuiLayer::Create();
        // PushOverlay(mGuiLayer);
        // mTimer.Start();
    }

    Application::~Application()
    {
        mTimer.Stop();
    }

    void Application::PushLayer(Layer *layer)
    {
        mLayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer *overlay)
    {
        mLayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }
    
    void Application::Run()
    {
        while (mRunning)
        {
            float time = window->Time();
            mTime = time - mLastFrameTime;
            mLastFrameTime = time;

            if (!mMinimized)
            {
                for (Layer *layer : mLayerStack)
                {
                    layer->OnUpdate();
                }

                // mGuiLayer->Begin();
                for (Layer *layer : mLayerStack)
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
        mRunning = false;
    }

    void Application::OnEvent(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNC(Application::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FUNC(Application::OnWindowResize));

        for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
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
        mRunning = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.Width() == 0 || e.Height() == 0)
        {
            mMinimized = true;
        }
        else
        {
            mMinimized = false;
            Renderer::OnWindowResize(e.Width(), e.Height());
        }

        return mMinimized;
    }
}
