#pragma once

#include "ImmortalCore.h"
#include "Timer.h"

#include "Immortal/Events/Event.h"
#include "Immortal/Events/ApplicationEvent.h"
#include "Immortal/Events/MouseEvent.h"
#include "Immortal/Events/KeyEvent.h"
#include "Immortal/Core/Window.h"
#include "Immortal/Core/LayerStack.h"
#include "Immortal/ImGui/GuiLayer.h"

#include "Immortal/Core/Input.h"

#include "Immortal/Render/VertexArray.h"
#include "Immortal/Render/Shader.h"
#include "Immortal/Render/Buffer.h"

#include "Immortal/Render/OrthographicCamera.h"

namespace Immortal {

	class IMMORTAL_API Application
	{
	public:
		struct Props
		{
			std::string Name;
			uint32_t Width;
			uint32_t Height;
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

		GuiLayer *GetGuiLayer() const NOEXCEPT
		{
			return mGuiLayer;
		}

		Window& GetWindow() const NOEXCEPT
		{
			return *mWindow;
		}

		void* GetNativeWindow() const NOEXCEPT
		{
			return mWindow->GetNativeWindow();
		}

		static bool IsKeyPressed(KeyCode code)
		{
			return Instance->_M_input.InternalIsKeyPressed(code);
		}

	public:
		static float Width() { return (float)Instance->mProps.Width; }
		static float Height() { return (float)Instance->mProps.Height; }
		static const char *Name() { return Instance->mProps.Name.c_str(); }
		static float DeltaTime() { return Instance->mTime.DeltaTime(); }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent &e);
	
	private:
		Scope<Window> mWindow;
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