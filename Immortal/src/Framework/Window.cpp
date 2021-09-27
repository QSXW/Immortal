#include "impch.h"
#include "Window.h"

#include "Platform/Windows/GLFWWindow.h"

namespace Immortal
{

std::unique_ptr<Window> Window::Create(const Description &description)
{
#ifdef WINDOWS
        return std::make_unique<GLFWWindow>(description);
#else
    SLASSERT(false && "Unknown platform!");
    return nullptr;
#endif
}

}
