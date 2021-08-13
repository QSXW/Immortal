#include "impch.h"
#include "RenderContext.h"

#include "Framework/Application.h"
#include <GLFW/glfw3.h>

#include "Image.h"

namespace Immortal
{
namespace Vulkan
{
	VkResult RenderContext::Status = VK_NOT_READY;

	std::unordered_map<const char *, bool> RenderContext::InstanceExtensions{
		{ IMMORTAL_PLATFORM_SURFACE, false }
	};

	std::unordered_map<const char *, bool> RenderContext::DeviceExtensions{
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, false }
	};

	static std::vector<const char *> ValidationLayers = {
		/*VK_LAYER_LUNARG_API_DUMP,
		VK_LAYER_LUNARG_DEVICE_SIMULATION,
		VK_LAYER_LAYER_LUNARG_ASSISTANT_LAYER,
		VK_LAYER_KHRONOS_VALIDATION,
		VK_LAYER_LUNARG_MONITOR,
		VK_LAYER_LUNARG_SCREENSHOT*/
	};

	RenderContext::RenderContext(RenderContext::Description &desc)
		: handle(desc.WindowHandle)
	{
		mInstance = MakeUnique<Instance>(Application::Name(), InstanceExtensions, ValidationLayers);
		this->CreateSurface();

		auto &gpu = mInstance->SuitableGraphicsProcessingUnit();
		gpu.RequestedFeatures.samplerAnisotropy = VK_TRUE;
		gpu.RequestedFeatures.robustBufferAccess = VK_TRUE;

		mDevice = MakeUnique<Device>(gpu, mSurface, DeviceExtensions);
		mQueue = &(mDevice->SuitableGraphicsQueue());

		mSurfaceExtent = VkExtent2D{ Application::Width(), Application::Height() };
		mSwapchain = MakeUnique<Swapchain>(*mDevice, mSurface, mSurfaceExtent);
	}

	RenderContext::~RenderContext()
	{
		vkDestroySurfaceKHR(mInstance->Handle(), mSurface, nullptr);
		mInstance.release();
	}

	void RenderContext::Init()
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

	void RenderContext::CreateSurface()
	{
		if (mInstance->Handle() == VK_NULL_HANDLE && !handle)
		{
			mSurface = VK_NULL_HANDLE;
			return;
		}

		Vulkan::Check(glfwCreateWindowSurface(mInstance->Handle(), static_cast<GLFWwindow *>(handle), nullptr, &mSurface));
	}

	void RenderContext::Prepare(size_t threadCount, RenderTarget::CreateFunc CreateRenderTargetfunc)
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
				auto swapchainImage = Image{ *mDevice, image, extent, mSwapchain->Format(), mSwapchain->Usage() };
				auto renderTarget = CreateRenderTargetfunc(std::move(swapchainImage));
			}
		}
	}

	void RenderContext::SwapBuffers()
	{

	}
}
}