#include "impch.h"
#include "Window.h"

#include "Platform/Windows/GLFWWindow.h"

namespace Immortal
{
    Scope<Window> Window::Create(const Description &description)
    {
    #ifdef WINDOWS
        #ifndef IMMORTAL_WINDOWS_DIRECT
            return CreateScope<GLFWWindow>(description);
        #else
            return CreateScope<DirectWindow>(description);
        #endif
    #else
        SLASSERT(false && "Unknown platform!");
        return nullptr;
    #endif
    }
}
