#include "GLFWWindow.h"
#include "GLFWInput.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"
#include "Shared/Log.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

namespace Immortal
{

static void GLFWErrorCallback(int error, const char *description)
{
    if (error == GLFW_FEATURE_UNAVAILABLE && glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
    {
        return;
    }
    LOG::ERR("GLFW Error ({0}): {1}", error, description);
}

GLFWWindow::GLFWWindow(Anonymous handle) :
    window{ (GLFWwindow *)handle },
    eventCallback{},
    parent{},
    childWindow{},
    input{},
    owned{}
{
	type = Type::GLFW;
}

GLFWWindow::GLFWWindow(const std::string &title, uint32_t width, uint32_t height, Window *parent) :
    window{},
    eventCallback{},
    parent{ (GLFWWindow *)parent },
    childWindow{},
    input{},
    owned{ true }
{
	if (parent)
	{
		SLASSERT(parent->GetType() != Type::GLFW && "Invalid parent window type!");
	}
    Construct(title, width, height);
}

GLFWWindow::~GLFWWindow()
{
    Shutdown();
}

uint32_t GLFWWindow::GetWidth() const
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return width;
}

uint32_t GLFWWindow::GetHeight() const
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return height;
}

void GLFWWindow::SetEventCallback(const EventCallbackFunc &callback)
{
    eventCallback = callback;
}

void GLFWWindow::SetTitle(const std::string &title)
{
    glfwSetWindowTitle(window, title.c_str());
}

void GLFWWindow::SetIcon(const std::string &filepath)
{
    // Picture picture = Vision::Read(filepath);
    // if (picture)
    // {
    //     GLFWimage image{};
    //     image.width  = (int)picture.GetWidth();
    //     image.height = (int)picture.GetHeight();
    //     image.pixels = (unsigned char *)picture.GetData();

    //     glfwSetWindowIcon(window, 1, &image);
    // }
}

void GLFWWindow::SelectPlatformType()
{
    int platform = glfwGetPlatform();
    switch (platform)
    {
        case GLFW_PLATFORM_WAYLAND:
            type = Type::Wayland;
            break;

        case GLFW_PLATFORM_X11:
            type = Type::XCB;
            break;

        case GLFW_PLATFORM_COCOA:
            type = Type::Cocoa;
            break;

        default:
            type = Type::GLFW;
            break;
    }
}

void GLFWWindow::Construct(const std::string &title, uint32_t width, uint32_t height)
{
    if (!parent)
    {
        glfwSetErrorCallback(GLFWErrorCallback);
#ifdef __linux__
        const char *session = getenv("XDG_SESSION_TYPE");
        if (!strcmp(session, "wayland"))
        {
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
        }
        else if(!strcmp(session, "x11"))
        {
            glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, true);
        }
#endif
        auto error = glfwInit();
        ThrowIf(!error, "Failed to initialize GLFW Window!");
    }

    SelectPlatformType();
    if (GetType() == Type::Cocoa)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    window = glfwCreateWindow((int)width, (int)height, title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);

    /* Set callbacks */
    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height)
    {
        GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));
        WindowResizeEvent event(width, height);
        This->eventCallback(event);
    });

    /* Set callbacks */
    glfwSetWindowPosCallback(window, [](GLFWwindow *window, int x, int y)
    {
        GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));
        WindowMoveEvent event(x, y);
        This->eventCallback(event);
    });

    glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
    {
        GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        This->eventCallback(event);
    });

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int modes)
    {
        GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));

        switch (action)
        {
            case GLFW_PRESS:
            {
                KeyPressedEvent event(key, 0);
                This->eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                This->eventCallback(event);
                break;
            }

            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, 1);
                This->eventCallback(event);
                break;
            }
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int modes)
    {
        GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));

        switch (action)
        {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event((MouseCode)button);
                This->eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event((MouseCode)button);
                This->eventCallback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset)
    {
		GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
		This->eventCallback(event);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xPos, double yPos)
    {
		GLFWWindow *This = (GLFWWindow *)(glfwGetWindowUserPointer(window));

        MouseMoveEvent event((float)xPos, (float)yPos);
		This->eventCallback(event);
    });

    input.reset(new GLFWInput{ this });
}

void GLFWWindow::Shutdown()
{
    if (!owned)
    {
		return;
    }

    glfwDestroyWindow(window);
	if (!parent)
	{
		glfwTerminate();
	}
}

Anonymous GLFWWindow::GetBackendHandle() const
{
    return (void *)window;
}

Anonymous GLFWWindow::GetPlatformSpecificHandle() const
{
#ifdef _WIN32
    return Anonymous((void *)glfwGetWin32Window(window));
#else
    return nullptr;
#endif
}

void GLFWWindow::Show()
{

}

void GLFWWindow::ProcessEvents()
{
    glfwPollEvents();
}

}
