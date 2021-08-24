#include "impch.h"
#include "OpenGLRenderContext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Immortal {

	OpenGLRenderContext::OpenGLRenderContext(RenderContext::Description &desc)
		: handle(static_cast<GLFWwindow*>(desc.WindowHandle))
	{
		SLASSERT(handle && "Window Handle is null!");
	}

	void OpenGLRenderContext::Init()
	{
		glfwMakeContextCurrent(handle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SLASSERT(status && "Failed to initialize Glad!");

		mGraphicsRenderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
		mDriverVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
		IM_CORE_INFO("Renderer: {0}", mGraphicsRenderer.c_str());
		IM_CORE_INFO("Vecdor: {0}", glGetString(GL_VENDOR));
		IM_CORE_INFO("Version: {0}", mDriverVersion.c_str());

		SLASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5) && "Immortal requires at least OpenGL version 4.5!");
	}

	void OpenGLRenderContext::SwapBuffers()
	{
		glfwSwapBuffers(handle);
	}

}


