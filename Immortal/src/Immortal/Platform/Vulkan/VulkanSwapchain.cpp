#include "impch.h"
#include "VulkanSwapchain.h"

namespace Immortal
{
	VulkanSwapchain::VulkanSwapchain(VulkanDevice                         &device,
		                             VkSurfaceKHR                         surface,
		                             const VkExtent2D                     &extent,
		                             const UINT32                         imageCount,
		                             const VkSurfaceTransformFlagBitsKHR  transform,
		                             const VkPresentModeKHR               presentMode,
		                             const std::set<VkImageUsageFlagBits> &imageUsageFlags):
		VulkanSwapchain{ *this, device, surface, extent, imageCount, transform, presentMode, imageUsageFlags }
	{

	}

	/**
	 * @brief Checking if a swap chain is available is not sufficient, because it may not actually be compatible
	 * with our window surface. Creating a swap chain also involves a lot more settings than instance and device
	 * creation, so we need to query for some more details before we're able to proceed.
	 *
	 * @check
	 *  - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
	 *  - Surface formats (pixel format, color space)
	 *  - Available presentation modes
	 *
	 */
	VulkanSwapchain::VulkanSwapchain(VulkanSwapchain                      &oldSwapchain,
		                             VulkanDevice                         &device,
		                             VkSurfaceKHR                         surface,
		                             const VkExtent2D                     &extent,
		                             const UINT32                         imageCount,
		                             const VkSurfaceTransformFlagBitsKHR  transform,
		                             const VkPresentModeKHR               presentMode,
		                             const std::set<VkImageUsageFlagBits> &imageUsageFlags) :
		mDevice{ device },
		mSurface{ surface },
		mPresentModePriorities{ oldSwapchain.mPresentModePriorities },
		mSurfaceFormatPriorities{ oldSwapchain.mSurfaceFormatPriorities }
	{
		VkPhysicalDevice &physicalDeviceHandle = mDevice.GraphicsProcessingUnit().Handle();
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		Vulkan::Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceHandle, mSurface, &surfaceCapabilities));

		UINT32 surfaceFormatCount{ 0U };
		Vulkan::Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, mSurface, &surfaceFormatCount, nullptr));
		mSurfaceFormats.resize(surfaceFormatCount);
		Vulkan::Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, mSurface, &surfaceFormatCount, mSurfaceFormats.data()));

		IM_CORE_INFO(LOGB("Surface 支持下列格式:", "Surface supports the following surface formats:"));
		for (auto &f : mSurfaceFormats)
		{
			IM_CORE_INFO("  \t{0}", Vulkan::ToString(f));
		}

		UINT32 presentModeCount{ 0U };
		Vulkan::Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, mSurface, &presentModeCount, nullptr));
		mPresentModes.resize(presentModeCount);
		Vulkan::Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, mSurface, &presentModeCount, mPresentModes.data()));

		IM_CORE_INFO(LOGB("Surface 支持下列显示模式:", "Surface supports the following present modes:"));
		for (auto &p : mPresentModes)
		{
			IM_CORE_INFO("  \t{0}", Vulkan::ToString(p));
		}
	}
}