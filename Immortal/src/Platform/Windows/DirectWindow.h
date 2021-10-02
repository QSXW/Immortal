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
        EventDispatcher = callback;
    }

    virtual void *GetNativeWindow() const override
    {
        return handle;
    }

    virtual void *PlatformNativeWindow() const override 
    {
        return handle;
    }

    virtual void Show() override;

    virtual void ProcessEvents();

    virtual void SetTitle(const std::string &title) override;

private:
    virtual void INIT(const Description &description);

    virtual void Shutdown();

private:
    HWND handle;

    WNDCLASSEX wc;

    Description desc;

public:
    static Window::EventCallbackFunc EventDispatcher;
};

}

