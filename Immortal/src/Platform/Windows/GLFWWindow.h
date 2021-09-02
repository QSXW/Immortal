#pragma once

#include "Framework/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Render/RenderContext.h"

namespace Immortal
{
    class GLFWWindow : public Window
    {
    public:
        GLFWWindow(const WindowProps& props);

        virtual ~GLFWWindow();

        void OnUpdate() override;

        UINT32 Width() const override
        {
            return description.Width;
        }

        UINT32 Height() const override
        {
            return description.Height;
        }

        void SetEventCallback(const EventCallbackFunc& callback) override
        {
            description.EventCallback = callback;
        }

        void SetVSync(bool enabled) override;
        bool IsVSync() const override;

        virtual void* GetNativeWindow() const
        {
            return  window;
        }

        virtual void* PlatformNativeWindow() const
        {
            return glfwGetWin32Window(window);
        }

        inline void Clear() override;

        float Time() override;

        virtual void ProcessEvents() override;

        virtual float DpiFactor() const override;
        virtual void SetTitle(const std::string &title) override;

    private:
        virtual void Init(const WindowProps& props);

        virtual void Shutdown();

    private:
        GLFWwindow* window;
        Unique<RenderContext> context;

        struct Description
        {
            std::string Title;

            UINT32 Width;
            UINT32 Height;
            bool Vsync;

            EventCallbackFunc EventCallback;
        } description{};

    public:
        static UINT8 GLFWWindowCount;
    };
}

