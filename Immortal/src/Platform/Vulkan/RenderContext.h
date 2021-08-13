#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace Vulkan
{
	class RenderContext : public Immortal::RenderContext
	{
		using Description = ::Immortal::RenderContext::Description;
	public:
		RenderContext() = default;
		RenderContext(Description &desc);
		~RenderContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;

		void CreateSurface();

	public:
		void Prepare(size_t threadCount = 1, RenderTarget::CreateFunc func = RenderTarget::DefaultCreateFunc);

	private:
		void *handle;
		Unique<Instance>  mInstance;
		Unique<Device>    mDevice;
		Unique<Swapchain> mSwapchain;

		VkSurfaceKHR   mSurface;
		VkExtent2D mSurfaceExtent;

		const Queue *mQueue;
		Swapchain::Properties mSwapchainProperties;

		std::vector<VkPresentModeKHR> mPresentModePriorities = {
			VK_PRESENT_MODE_FIFO_KHR,
			VK_PRESENT_MODE_MAILBOX_KHR
		};

		std::vector<VkSurfaceFormatKHR> mSurfaceFormatPriorities = {
			{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
		};

		size_t mThreadCount{ 1 };
	public:
		static VkResult Status;
		static std::unordered_map<const char *, bool> InstanceExtensions;
		static std::unordered_map<const char *, bool> DeviceExtensions;
	};
}
}
