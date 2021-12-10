#include "impch.h"
#include "Window.h"

#include "Platform/Windows/GLFWWindow.h"
#include "Platform/Windows/DirectWindow.h"
#include "Render/Render.h"

namespace Immortal
{

std::unique_ptr<Window> Window::Create(const Description &description)
{
#ifdef WINDOWS
    if (Render::API == Render::Type::D3D12)
    {
        LOG::INFO("Creating window with native window");
        return std::make_unique<DirectWindow>(description);
    }
    LOG::INFO("Creating window with glfw");
    return std::make_unique<GLFWWindow>(description);
#else
    SLASSERT(false && "Unknown platform!");
    return nullptr;
#endif
}

}
