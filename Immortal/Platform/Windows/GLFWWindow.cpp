#include "GLFWWindow.h"
#include "GLFWInput.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"
#include "Render/Render.h"
#include "Render/Frame.h"

namespace Immortal
{

uint8_t GLFWWindow::GLFWWindowCount = 0;

static void GLFWErrorCallback(int error, const char *description)
{
    LOG::ERR("GLFW Error ({0}): {1}", error, description);
}

GLFWWindow::GLFWWindow(const Description &description)
{
    Setup(description);
}

GLFWWindow::~GLFWWindow()
{
    Shutdown();
}

/// @brief It calculates the dpi factor using the density from GLFW physical size
/// <a href="https://www.glfw.org/docs/latest/monitor_guide.html#monitor_size">GLFW docs for dpi</a>
float GLFWWindow::DpiFactor() const
{
    auto primaryMonitor = glfwGetPrimaryMonitor();
    auto videoMode = glfwGetVideoMode(primaryMonitor);

    INT32 mmWidth;
    INT32 mmHeight;
    glfwGetMonitorPhysicalSize(primaryMonitor, &mmWidth, &mmHeight);

    static constexpr const float inch2mm = 25.0f;
    static constexpr const float windowBaseDesity = 96.0f;

    auto dpi = static_cast<UINT32>(videoMode->width / (mmWidth / inch2mm));
    return dpi / windowBaseDesity;
}

void GLFWWindow::SetTitle(const std::string &title)
{
    desc.Title = title;
    glfwSetWindowTitle(window, desc.Title.c_str());
}

void GLFWWindow::SetIcon(const std::string &filepath)
{
    Frame frame{ filepath };
    Vision::Picture pic = frame.GetPicture();

    GLFWimage image{};
    image.height = ncast<int>(pic.desc.height);
    image.width  = ncast<int>(pic.desc.width);
    image.pixels = rcast<unsigned char *>(frame.Data());

    glfwSetWindowIcon(window, 1, &image);
}

void GLFWWindow::Setup(const Description &description)
{
    desc = description;

    LOG::INFO("Creating window {} ({}, {})", desc.Title, desc.Width, desc.Height);

    if (GLFWWindowCount == 0)
    {
        glfwSetErrorCallback(GLFWErrorCallback);
        auto error = glfwInit();
        ThrowIf(!error, "Failed to initialize GLFW Window!");
    }

    if (Render::API == Render::Type::Vulkan || Render::API == Render::Type::D3D12)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    window = glfwCreateWindow((int)desc.Width, (int)desc.Height, description.Title.c_str(), nullptr, nullptr);
    GLFWWindowCount++;

    glfwSetWindowUserPointer(window, &desc);

    /* Set callbacks */
    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));
        desc->Width = width;
        desc->Height = height;

        WindowResizeEvent event(width, height);
        desc->EventCallback(event);
    });

    glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        desc->EventCallback(event);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int modes)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));

        switch (action)
        {
            case GLFW_PRESS:
            {
                KeyPressedEvent event(key, 0);
                desc->EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                desc->EventCallback(event);
                break;
            }

            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, 1);
                desc->EventCallback(event);
                break;
            }
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int modes)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));

        switch (action)
        {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event((MouseCode)button);
                desc->EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event((MouseCode)button);
                desc->EventCallback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        desc->EventCallback(event);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xPos, double yPos)
    {
        Description *desc = bcast<Description *>(glfwGetWindowUserPointer(window));

        MouseMoveEvent event((float)xPos, (float)yPos);
        desc->EventCallback(event);
    });

    input.reset(new GLFWInput{ this });
}

void GLFWWindow::Shutdown()
{
    glfwDestroyWindow(window);
    --GLFWWindowCount;

    if (GLFWWindowCount == 0)
    {
        glfwTerminate();
    }
}

Anonymous GLFWWindow::Primitive() const
{
    return Anonymize(window);
}

float GLFWWindow::Time() const
{
    return ncast<float>(glfwGetTime());
}

}
