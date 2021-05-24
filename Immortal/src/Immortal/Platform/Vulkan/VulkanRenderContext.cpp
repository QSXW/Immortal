#include "impch.h"
#include "VulkanRenderContext.h"

#include "Immortal/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Immortal
{
	std::unordered_map<const char *, bool> VulkanRenderContext::InstanceExtensions{ { IMMORTAL_PLATFORM_SURFACE, false } };
	std::unordered_map<const char *, bool> VulkanRenderContext::DeviceExtensions{ { VK_KHR_SWAPCHAIN_EXTENSION_NAME, false } };

	/**
	 * @breif Vulkan is a platform agnostic API, which means that we need
	 * an extension to inteface with the window system.
	 */
	VulkanRenderContext::VulkanRenderContext(RenderContext::Description &desc)
		: mHandle(desc.WindowHandle)
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

		this->CreateSurface();

		auto &gpu = mInstance->SuitableGraphicsProcessingUnit();
		gpu.RequestedFeatures.samplerAnisotropy  = VK_TRUE;
		gpu.RequestedFeatures.robustBufferAccess = VK_TRUE;

		mDevice = MakeUnique<VulkanDevice>(gpu, mSurface, DeviceExtensions);
	}

	VulkanRenderContext::~VulkanRenderContext()
	{
		vkDestroySurfaceKHR(mInstance->Handle(), mSurface, nullptr);
		mInstance.release();
	}

	void VulkanRenderContext::Init()
	{

	}

	void VulkanRenderContext::CreateSurface()
	{
		if (mInstance->Handle() == VK_NULL_HANDLE && !mHandle)
		{
			mSurface = VK_NULL_HANDLE;
			return;
		}

		Vulkan::Check(glfwCreateWindowSurface(mInstance->Handle(), static_cast<GLFWwindow *>(mHandle), nullptr, &mSurface));
	}

	void VulkanRenderContext::SwapBuffers()
	{

	}

}