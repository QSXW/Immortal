#pragma once

#include "ImmortalCore.h"
#include "Timer.h"

#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"
#include "Framework/Window.h"
#include "Framework/LayerStack.h"
#include "ImGui/GuiLayer.h"

#include "Framework/Input.h"

#include "Render/VertexArray.h"
#include "Render/Shader.h"
#include "Render/Buffer.h"

#include "Render/OrthographicCamera.h"

namespace Immortal {

	class IMMORTAL_API Application
	{
	public:
		struct Props
		{
			std::string Name;
			UINT32 Width;
			UINT32 Height;
		};

	public:
		Application(const Props &props = { "Immortal Engine", 1920, 1080 });
		virtual ~Application();

		virtual void Run();
		virtual void Close();

		void OnEvent(Event &e);

		virtual inline void PushLayer(Layer *layer);
		virtual inline void PushOverlay(Layer *overlay);

		static Application *App()
		{
			return Instance;
		}

		virtual GuiLayer *GetGuiLayer() const
		{
			return mGuiLayer;
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
			return Instance->_M_input.InternalIsKeyPressed(code);
		}

	public:
		static UINT32 Width() { return Instance->mProps.Width; }
		static UINT32 Height() { return Instance->mProps.Height; }
		static const char *Name() { return Instance->mProps.Name.c_str(); }
		static float DeltaTime() { return Instance->mTime.DeltaTime(); }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent &e);
	
	private:
		Scope<Window> window;
		bool mRunning = true;
		bool mMinimized = false;
		LayerStack mLayerStack;
		GuiLayer *mGuiLayer;
		float mLastFrameTime{ 0.0f };

		Props mProps;

		Input _M_input;
	public:
		static Application *Instance;
	private:
		Timestep mTime;
		Timer    mTimer;
	};

	/* To be defined in Client */
	Application* CreateApplication();
}