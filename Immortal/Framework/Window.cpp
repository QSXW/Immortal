#include "Window.h"

#include "Render/Render.h"
#include "Platform/Windows/GLFWWindow.h"

#ifdef _WIN32
#include "Platform/Windows/DirectWindow.h"
#endif

namespace Immortal
{

Window *Window::Create(const Description &description)
{
#ifdef _WIN32
    if (Render::API & Render::Type::D3D)
    {
        LOG::INFO("Creating window with Native Win32");
        return new DirectWindow{ description };
    }
#endif
    if (Render::API & (Render::Type::Vulkan | Render::Type::OpenGL))
    {
        LOG::INFO("Creating window with GLFW");
        return new GLFWWindow{ description };
    }

    LOG::WARN("There is no Window implemented for the rendering API specified yet.");
    return nullptr;
}

}
