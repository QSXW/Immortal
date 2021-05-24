#pragma once

#include "Immortal/Core/Window.h"
#include "Immortal/Render/RenderContext.h"

namespace Immortal
{
	class IMMORTAL_API DirectWindow : public Window
	{
	public:
		DirectWindow(const WindowProps& props);
		virtual ~DirectWindow();

		void OnUpdate() override;

		uint32_t Width() const override
		{
			return m_Data.Width;
		}
		uint32_t Height() const override
		{
			return m_Data.Height;
		}

		void SetEventCallback(const EventCallbackFunc& callback) override
		{
			m_Data.EventCallback = callback;
		}

		void SetVSync(bool enabled) override;
		inline bool IsVSync() const override;

		virtual void* GetNativeWindow() const
		{
			return m_hwnd;
		}

		virtual void ProcessEvents();

		inline void Clear() override;

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		HWND m_hwnd;
		WNDCLASSEX m_WindowClass;
		Scope<RenderContext> m_Context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool Vsync;

			EventCallbackFunc EventCallback;
		} m_Data{ };
	};

}

