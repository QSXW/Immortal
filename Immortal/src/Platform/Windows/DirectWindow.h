#pragma once

#include "Framework/Window.h"
#include "Render/RenderContext.h"

#ifndef UNICODE
#define UNICODE
#endif

namespace Immortal
{
	class IMMORTAL_API DirectWindow : public Window
	{
	public:
		DirectWindow(const Description &description);

		virtual ~DirectWindow();

		void OnUpdate() override;

		virtual UINT32 Width() const override
		{
			return desc.Width;
		}

		virtual UINT32 Height() const override
		{
			return desc.Height;
		}

		virtual void SetEventCallback(const EventCallbackFunc& callback) override
		{
			desc.EventCallback = callback;
		}

		virtual void SetVSync(bool enabled) override;

		virtual bool IsVSync() const override
		{
			return desc.Vsync;
		}

		virtual void* GetNativeWindow() const
		{
			return handle;
		}

		virtual void ProcessEvents();

		inline void Clear() override;

	private:
		virtual void Init(const Description &description);

		virtual void Shutdown();

	private:
		HWND handle;

		WNDCLASSEX wc;

		Description desc;
	};
}

