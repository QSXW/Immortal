#pragma once

#include "Framework/Window.h"
#include "Framework/Input.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Immortal
{

class GLFWWindow : public Window
{
public:
    GLFWWindow(const Description &description);

    virtual ~GLFWWindow();

    uint32_t Width() const override
    {
        return desc.Width;
    }

    uint32_t Height() const override
    {
        return desc.Height;
    }

    void SetEventCallback(const EventCallbackFunc& callback) override
    {
        desc.EventCallback = callback;
    }

    virtual void *GetNativeWindow() const
    {
        return window;
    }

    virtual void *Primitive() const override;

    float Time() const override;

    virtual void ProcessEvents() override
    {
        glfwPollEvents();
    }

    virtual float DpiFactor() const override;

    virtual void SetTitle(const std::string &title) override;

    virtual void SetIcon(const std::string &filepath) override;

private:
    virtual void Setup(const Description &descrition);

    virtual void Shutdown();

private:
    GLFWwindow *window;

    Description desc{};

    std::unique_ptr<Input> input;

public:
    static uint8_t GLFWWindowCount;
};
}
