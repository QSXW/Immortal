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

	DirectWindow(const std::string &title, uint32_t width, uint32_t height);

    virtual ~DirectWindow();

    virtual uint32_t GetWidth() const override;

    virtual uint32_t GetHeight() const override;

    virtual void SetEventCallback(const EventCallbackFunc &callback) override;

    virtual Anonymous GetBackendHandle() const override;

    virtual Anonymous GetPlatformSpecificHandle() const override;

    virtual void Show() override;

    virtual void SetTitle(const std::string &title) override;

    virtual void SetIcon(const std::string &filepath) override;

    virtual void ProcessEvents() override;

protected:
	void Construct(const std::string &title, uint32_t width, uint32_t height);

    void Shutdown();

protected:
    HWND handle;

    WNDCLASSEXW wc;

    bool owned;

public:
    static Window::EventCallbackFunc EventDispatcher;

    static std::unique_ptr<NativeInput> Input;
};

}
