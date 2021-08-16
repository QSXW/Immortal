#include "impch.h"
#include "Renderer.h"

namespace Immortal
{
namespace Vulkan {
	static std::vector<VkSurfaceFormatKHR> SurfacePriorityList = {
		{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
	    { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
	    {  VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
	    {  VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
	};

	void Renderer::Init()
	{

	}
}
}