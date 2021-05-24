#include "impch.h"
#include "Application.h"

#include "Immortal/Events/Event.h"
#include "Immortal/Events/ApplicationEvent.h"
#include "Immortal/Core/Log.h"

#include "Immortal/Render/Renderer.h"
#include "Input.h"

namespace Immortal {

#define BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)

	Application *Application::Instance = nullptr;

	Application::Application(const Props &props)
	{
		if (Instance)
		{
			return;
		}
		Instance = this;
		mProps = props;
		mWindow = Window::Create({ props.Name, props.Width, props.Height });
		mWindow->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));
		mWindow->SetVSync(false);

		// Renderer::Init();
		//mGuiLayer = GuiLayer::Create();
		//PushOverlay(mGuiLayer);

		mTimer.Start();
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
			float time = mWindow->Time();
			mTime = time - mLastFrameTime;
			mLastFrameTime = time;

			if (!mMinimized)
			{
				for (Layer *layer : mLayerStack)
				{
					layer->OnUpdate();
				}

				mGuiLayer->Begin();
				for (Layer *layer : mLayerStack)
				{
					layer->OnGuiRender();
				}
				mGuiLayer->End();
			}
			mWindow->OnUpdate();
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
