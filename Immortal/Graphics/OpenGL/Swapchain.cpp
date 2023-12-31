#include "Swapchain.h"
#include "Common.h"
#include "RenderTarget.h"

#include <GLFW/glfw3.h>

namespace Immortal
{
namespace OpenGL
{

Swapchain::Swapchain(Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) :
    window{ window },
    width{ window->GetWidth() },
    height{ window->GetHeight() },
    renderTarget{}
{
	glfwSwapInterval((int)mode);
	renderTarget.SetWidth(width);
	renderTarget.SetHeight(height);
}

Swapchain::~Swapchain()
{
	renderTarget = {};
	window = nullptr;
}

void Swapchain::PrepareNextFrame()
{
	glfwMakeContextCurrent((GLFWwindow *) window->GetBackendHandle());
}

void Swapchain::Resize(uint32_t _width, uint32_t _height)
{
	width  = _width;
	height = _height;
	renderTarget.SetWidth(width);
	renderTarget.SetHeight(height);
}

SuperRenderTarget *Swapchain::GetCurrentRenderTarget()
{
	return &renderTarget;
};

void Swapchain::Present()
{
	glfwSwapBuffers((GLFWwindow *)window->GetBackendHandle());
}

void Swapchain::SetWindowContext(bool trigger)
{
	if (trigger)
	{
		glfwMakeContextCurrent((GLFWwindow *) window->GetBackendHandle());
	}
	else
	{
		glfwMakeContextCurrent(nullptr);
	}
}

}

}
