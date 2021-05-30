#pragma once

#include "Immortal/Render/RenderContext.h"

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderTarget.h"

namespace Immortal
{
	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext() = default;
		VulkanRenderContext(RenderContext::Description &desc);
		~VulkanRenderContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;

		void CreateSurface();

	public:
		void Prepare(size_t threadCount = 1, VulkanRenderTarget::CreateFunc func = VulkanRenderTarget::DefaultCreateFunc);

	private:
		void *mHandle;
		Unique<VulkanInstance>  mInstance;
		Unique<VulkanDevice>    mDevice;
		Unique<VulkanSwapchain> mSwapchain;

		VkSurfaceKHR   mSurface;
		VkExtent2D mSurfaceExtent;

		const VulkanQueue *mQueue;
		VulkanSwapchain::Properties mSwapchainProperties;

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
		static std::unordered_map<const char *, bool> InstanceExtensions;
		static std::unordered_map<const char *, bool> DeviceExtensions;
	};
}

