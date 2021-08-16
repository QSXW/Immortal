#pragma once

#include "ImmortalCore.h"
#include "Render/RenderContext.h"

struct GLFWwindow;
namespace Immortal {

	class OpenGLRenderContext : public RenderContext
	{
	public:
		OpenGLRenderContext(RenderContext::Description &desc);

		void Init() override;
		void SwapBuffers() override;

	private:
		GLFWwindow* handle;
	};

}

