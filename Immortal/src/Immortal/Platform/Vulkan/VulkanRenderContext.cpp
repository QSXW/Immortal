#include "impch.h"
#include "VulkanRenderContext.h"

#include "Immortal/Core/Application.h"

namespace Immortal
{
	std::unordered_map<const char *, bool> VulkanRenderContext::InstanceExtensions{ { IMMORTAL_PLATFORM_SURFACE, false } };

	/**
	 * @breif Vulkan is a platform agnostic API, which means that we need
	 * an extension to inteface with the window system.
	 */
	VulkanRenderContext::VulkanRenderContext(RenderContext::Description &desc)
	{
		std::vector<const char *> validationLayers = {
#if			IMMORTAL_CHECK_DEBUG
			// "VK_LAYER_LUNARG_api_dump",
			// "VK_LAYER_LUNARG_device_simulation",
			"VK_LAYER_KHRONOS_synchronization2",
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor"
#endif
		};
		mInstance = MakeUnique<VulkanInstance>(Application::Name(), InstanceExtensions, validationLayers);

		auto &gpu = mInstance->SuitableGraphicsProcessingUnit();
	}

	VulkanRenderContext::~VulkanRenderContext()
	{

	}

	void VulkanRenderContext::Init()
	{

	}

	void VulkanRenderContext::SwapBuffers()
	{

	}

}