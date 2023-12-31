#include "Window.h"

#include "Window/GLFWWindow.h"

#ifdef _WIN32
#include "Window/DirectWindow.h"
#endif

namespace Immortal
{

Window *Window::CreateInstance(Anonymous handle, WindowType type)
{
#ifdef _WIN32
	if (type != WindowType::GLFW)
	{
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
        return new DirectWindow{ title, width, height };
    }
#endif
    return new GLFWWindow{ title, width, height };

    LOG::WARN("There is no Window implemented for the rendering API specified yet.");
    return nullptr;
}

}
