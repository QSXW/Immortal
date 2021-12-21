#pragma once

#include "Framework/Window.h"
#include "Render/RenderContext.h"
#include "Framework/Input.h"

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

    virtual uint32_t Width() const override
    {
        return desc.Width;
    }

    virtual uint32_t Height() const override
    {
        return desc.Height;
    }

    virtual void SetEventCallback(const EventCallbackFunc &callback) override
    {
        EventDispatcher = callback;
    }

    virtual void *GetNativeWindow() const override
    {
        return handle;
    }

    virtual void *Primitive() const override;

    virtual void Show() override;

    virtual void SetTitle(const std::string &title) override;

    virtual void SetIcon(const std::string &filepath) override;

    virtual void ProcessEvents() override;

private:
    virtual void Setup(const Description &description);

    virtual void Shutdown();

private:
    HWND handle;

    WNDCLASSEX wc;

    Description desc;

    std::unique_ptr<Input> input;

public:
    static Window::EventCallbackFunc EventDispatcher;
};

}
