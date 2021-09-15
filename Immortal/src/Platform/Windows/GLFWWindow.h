#pragma once

#include "Framework/Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Render/RenderContext.h"

namespace Immortal
{
class GLFWWindow : public Window
{
public:
    GLFWWindow(const Description &description);

    virtual ~GLFWWindow();

    void OnUpdate() override;

    UINT32 Width() const override
    {
        return desc.Width;
    }

    UINT32 Height() const override
    {
        return desc.Height;
    }

    void SetEventCallback(const EventCallbackFunc& callback) override
    {
        desc.EventCallback = callback;
    }

    virtual void SetVSync(bool enabled) override;

    virtual bool IsVSync() const override;

    virtual void* GetNativeWindow() const
    {
        return  window;
    }

    virtual void* PlatformNativeWindow() const
    {
        return glfwGetWin32Window(window);
    }

    float Time() override;

    virtual void ProcessEvents() override;

    virtual float DpiFactor() const override;

    virtual void SetTitle(const std::string &title) override;

private:
    virtual void Init(const Description &descrition);

    virtual void Shutdown();

private:
    GLFWwindow *window;

    Description desc{};

public:
    static UINT8 GLFWWindowCount;
};
}
