#pragma once

#include "Window.h"
#include "NativeInput.h"

#ifndef UNICODE
#define UNICODE
#endif

namespace Immortal
{

class IMMORTAL_API DirectWindow : public Window
{
public:
	DirectWindow(Anonymous handle);

	DirectWindow(const std::string &title, uint32_t width, uint32_t height, bool borderlessWindow = false);

    virtual ~DirectWindow();

    virtual uint32_t GetWidth() const override;

    virtual uint32_t GetHeight() const override;

    virtual void SetEventCallback(const EventCallbackFunc &callback) override;

    virtual Anonymous GetBackendHandle() const override;

    virtual Anonymous GetPlatformSpecificHandle() const override;

    virtual void Show() override;

    virtual void SetFullscreen(bool value) override;

    virtual bool IsFullscreen() const override;

    virtual void SetTitle(const std::string &title) override;

    virtual void SetIcon(const std::string &filepath) override;

    virtual void ProcessEvents() override;

	void CaptionButtonMinimize() override;

	void CaptionButtonMaximizeOrRestore() override;

	bool IsBorderless() const
	{
		return borderless;
	}

	static void SetBorderlessCaptionPreferClient(bool preferClientArea);

	static Window::EventCallbackFunc EventDispatcher;

	static std::unique_ptr<NativeInput> Input;

protected:
	void Construct(const std::string &title, uint32_t width, uint32_t height);

    void Shutdown();

    void CacheWindowedState();

	void ReleaseWindowIcon();

protected:
    HWND handle;

	HICON windowIcon = nullptr;

    WNDCLASSEXW wc;

    bool owned;

    bool fullscreen;

	bool zoomedBeforeExclusiveFullscreen = false;

    DWORD windowedStyle;

    DWORD windowedExStyle;

    WINDOWPLACEMENT windowedPlacement;

	bool borderless = false;
};

}
