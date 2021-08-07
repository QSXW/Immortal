#include "impch.h"
#include "VulkanRenderContext.h"

#include "Immortal/Core/Application.h"
#include <GLFW/glfw3.h>

#include "VulkanImage.h"

namespace Immortal
{
	std::unordered_map<const char *, bool> VulkanRenderContext::InstanceExtensions{
		{ IMMORTAL_PLATFORM_SURFACE, false }
	};

	/**
	 * @brief Not all graphics cards are capable of presenting images directly to a screen for various reasons,
	 * for example because they are designed for servers and don't have any display outputs. Secondly, since
	 * image presentation is heavily tied into the window system and the surfaces associated with windows, it 
	 * is not actually part of the Vulkan core. We have to enable the VK_KHR_swapchain device for RenderContext.
	 * 
	 */
	std::unordered_map<const char *, bool> VulkanRenderContext::DeviceExtensions{ 
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, false }
	};

	/**
	 * @breif Vulkan is a platform agnostic API, which means that we need
	 * an extension to inteface with the window system.
	 */
	VulkanRenderContext::VulkanRenderContext(RenderContext::Description &desc)
		: mHandle(desc.WindowHandle)
	{
		static std::vector<const char *> validationLayers = {
#if			IMMORTAL_CHECK_DEBUG
			/*"VK_LAYER_LUNARG_api_dump",
			"VK_LAYER_LUNARG_device_simulation",
			"VK_LAYER_LUNARG_assistant_layer",
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor",
			"VK_LAYER_LUNARG_screenshot"*/
#endif
		};

		mInstance = MakeUnique<VulkanInstance>(Application::Name(), InstanceExtensions, validationLayers);

		this->CreateSurface();

		auto &gpu = mInstance->SuitableGraphicsProcessingUnit();
		gpu.RequestedFeatures.samplerAnisotropy  = VK_TRUE;
		gpu.RequestedFeatures.robustBufferAccess = VK_TRUE;

		mDevice = MakeUnique<VulkanDevice>(gpu, mSurface, DeviceExtensions);
		mQueue = &(mDevice->SuitableGraphicsQueue());

		mSurfaceExtent = VkExtent2D{ Application::Width(), Application::Height() };
		mSwapchain = MakeUnique<VulkanSwapchain>(*mDevice, mSurface, mSurfaceExtent);
	}

	VulkanRenderContext::~VulkanRenderContext()
	{
		vkDestroySurfaceKHR(mInstance->Handle(), mSurface, nullptr);
		mInstance.release();
	}

	void VulkanRenderContext::Init()
	{
		mPresentModePriorities = {
			VK_PRESENT_MODE_MAILBOX_KHR,
			VK_PRESENT_MODE_IMMEDIATE_KHR,
			VK_PRESENT_MODE_FIFO_KHR
		};
	
		mSurfaceFormatPriorities = {
			{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
		};

		if (mSwapchain)
		{
			// This is commonly known as "triple buffering",
			mSwapchain->Set(VK_PRESENT_MODE_MAILBOX_KHR);
			mSwapchain->Set(VK_FORMAT_B8G8R8A8_UNORM);
		}
		this->Prepare();
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

	void VulkanRenderContext::Prepare(size_t threadCount, VulkanRenderTarget::CreateFunc CreateRenderTargetfunc)
	{
		mDevice->WaitIdle();

		if (mSwapchain)
		{
			mSwapchain->Set(mPresentModePriorities);
			mSwapchain->Set(mSurfaceFormatPriorities);
			mSwapchain->Create();

			VkExtent3D extent{ mSurfaceExtent.width, mSurfaceExtent.height, 1 };

			for (auto &image : mSwapchain->Images())
			{
				auto swapchainImage = VulkanImage{ *mDevice, image, extent, mSwapchain->Format(), mSwapchain->Usage() };
				auto renderTarget = CreateRenderTargetfunc(std::move(swapchainImage));
			}
		}
	}

	void VulkanRenderContext::SwapBuffers()
	{

	}

}