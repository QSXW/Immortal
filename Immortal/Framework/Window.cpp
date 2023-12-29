#include "Window.h"

#include "Render/Graphics.h"
#include "Platform/Windows/GLFWWindow.h"

#ifdef _WIN32
#include "Platform/Windows/DirectWindow.h"
#endif

namespace Immortal
{

Window *Window::CreateInstance(Anonymous handle, WindowType type)
{
#ifdef _WIN32
	if (type != WindowType::GLFW)
	{
		LOG::INFO("Creating window with Native Win32");
		return new DirectWindow{ handle };
	}
#endif
	return new GLFWWindow{ handle };
}

Window *Window::CreateInstance(const std::string &title, uint32_t width, uint32_t height, WindowType type)
{
#ifdef _WIN32
	if (type != WindowType::GLFW)
    {
        LOG::INFO("Creating window with Native Win32");
        return new DirectWindow{ title, width, height };
    }
#endif
    LOG::INFO("Creating window with GLFW");
    return new GLFWWindow{ title, width, height };

    LOG::WARN("There is no Window implemented for the rendering API specified yet.");
    return nullptr;
}

}
