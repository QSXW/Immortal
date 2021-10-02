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
        return std::make_unique<DirectWindow>(description);
    }
    return std::make_unique<GLFWWindow>(description);
#else
    SLASSERT(false && "Unknown platform!");
    return nullptr;
#endif
}

}
