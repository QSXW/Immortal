#pragma once

#include "ImmortalCore.h"
#include "Render/RenderContext.h"
#include "Framework/Device.h"

struct GLFWwindow;
namespace Immortal {

	class OpenGLRenderContext : public RenderContext
	{
	public:
		OpenGLRenderContext(RenderContext::Description &desc);

		void Init() override;

		void SwapBuffers() override;

		Device *GetDevice() override
		{
			return nullptr;
		}

	private:
		GLFWwindow* handle;
	};

}

