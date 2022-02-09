#include "impch.h"
#include "Window.h"

#include "Platform/Windows/GLFWWindow.h"
#include "Platform/Windows/DirectWindow.h"
#include "Render/Render.h"

namespace Immortal
{

Window *Window::Create(const Description &description)
{
#ifdef WINDOWS
    if (Render::API == Render::Type::D3D12)
    {
        LOG::INFO("Creating window with Native Win32");
        return new DirectWindow{ description };
    }
    if (Render::API & (Render::Type::Vulkan | Render::Type::OpenGL))
    {
        LOG::INFO("Creating window with GLFW");
        return new GLFWWindow{ description };
    }
    LOG::ERR("There is no Window implemented for the rendering API specified yet.");
    return nullptr;
#else
    THROWIF(true, "Unknown platform!");
    return nullptr;
#endif
}

}
