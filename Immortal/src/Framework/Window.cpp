#include "impch.h"
#include "Window.h"

#ifdef WIN32
	#include "Platform/Windows/GLFWWindow.h"
#endif

namespace Immortal
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
	#ifdef IMMORTAL_PLATFORM_WINDOWS
		#ifndef IMMORTAL_WINDOWS_DIRECT
			return CreateScope<GLFWWindow>(props);
		#else
			return CreateScope<DirectWindow>(props);
		#endif
	#else
		IM_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
	#endif
	}

}