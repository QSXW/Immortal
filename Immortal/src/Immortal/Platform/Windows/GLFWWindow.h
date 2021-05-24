#pragma once

#include "Immortal/Core/Window.h"


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Immortal/Render/RenderContext.h"


namespace Immortal
{
	class GLFWWindow : public Window
	{
	public:
		GLFWWindow(const WindowProps& props);
		virtual ~GLFWWindow();

		void OnUpdate() override;

		uint32_t Width() const override
		{
			return mData.Width;
		}
		uint32_t Height() const override
		{
			return mData.Height;
		}

		void SetEventCallback(const EventCallbackFunc& callback) override
		{
			mData.EventCallback = callback;
		}

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual void* GetNativeWindow() const
		{
			return  mWindow;
		}

		virtual void* PlatformNativeWindow() const
		{
			return glfwGetWin32Window(mWindow);
		}

		inline void Clear() override;
		float Time() override;

		virtual const RenderContext &Context() const override
		{
			return *mContext;
		}

		virtual void ProcessEvents() override;

		virtual float DpiFactor() const override;
		virtual void SetTitle(const std::string &title) override;

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* mWindow;
		Unique<RenderContext> mContext;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool Vsync;

			EventCallbackFunc EventCallback;
		} mData{};

	public:
		static UINT8 GLFWWindowCount;
	};

}

