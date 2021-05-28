#pragma once

#include "ImmortalCore.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace Immortal
{
	class VulkanSwapchain
	{
	public:
		struct Properties
		{
			VkSwapchainKHR                OldSwapchain;
			UINT32                        ImageCount{ 3 };
			VkExtent2D                    SurfaceFormat{};
			UINT32                        ArrayLayers;
			VkImageUsageFlags		      ImageUsage;
			VkSurfaceTransformFlagBitsKHR PreTransform;
			VkCompositeAlphaFlagBitsKHR   CompositeAlpha;
			VkPresentModeKHR              PresentMode;
		};

	public:


	private:
		VulkanDevice &mDevice;
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		VkSwapchainKHR mHandle{ VK_NULL_HANDLE };

		std::vector<VkImage> mImages;
		std::vector<VkSurfaceFormatKHR> mSurfaceFormats{};
		std::vector<VkPresentModeKHR> mPresentModes{};

		VulkanSwapchain::Properties mProperties;

		// A list of present modes in order of priority (index is the priority 0 - hight, And so on.)
		std::vector<VkPresentModeKHR> mPresentModePriorities = {
			VK_PRESENT_MODE_FIFO_KHR,
			VK_PRESENT_MODE_MAILBOX_KHR
		};

		// A list of surface formats in order of priority (index is the priority 0 - hight, And so on.)
		std::vector<VkSurfaceFormatKHR> mSurfaceFormatPriorities = {
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
		};

		VkPresentModeKHR mPresentMode{};
		
		std::set<VkImageUsageFlagBits> mImageUsageFlags;
	};
}

