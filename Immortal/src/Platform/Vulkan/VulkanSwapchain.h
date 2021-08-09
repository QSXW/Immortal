#pragma once

#include "ImmortalCore.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace Immortal
{
	/**
	 * @brief The swap chain is essentially a queue of images that are waiting to be presented to the screen.
	 */
	class VulkanSwapchain
	{
	public:

		/**
		 * @ArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application.
		 * @ImageUsage  specifies what kind of operations we'll use the images in the swap chain for.
		 */
		struct Properties
		{
			VkSwapchainKHR                OldSwapchain;
			UINT32                        ImageCount{ 3 };
			VkExtent2D                    Extent{};
			VkSurfaceFormatKHR            SurfaceFormat{};
			UINT32                        ArrayLayers{ 1 };
			VkImageUsageFlags		      ImageUsage;
			VkSurfaceTransformFlagBitsKHR PreTransform;
			VkCompositeAlphaFlagBitsKHR   CompositeAlpha;
			VkPresentModeKHR              PresentMode;
		};

	public:
		VulkanSwapchain(VulkanDevice                         &device,
			            VkSurfaceKHR                         surface,
			            const VkExtent2D                     &extent          = {},
			            const UINT32                         imageCount       = 3,
			            const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			            const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
			            const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT});

		VulkanSwapchain(VulkanSwapchain                      &oldSwapchain,
			            VulkanDevice                         &device,
			            VkSurfaceKHR                         surface,
			            const VkExtent2D                     &extent          = {},
			            const uint32_t                       imageCount       = 3,
			            const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			            const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
			            const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

		~VulkanSwapchain();

		void Create();

	public:
		void Set(const VkPresentModeKHR mode)
		{
			mProperties.PresentMode = mode;
		}

		void Set(const VkFormat format)
		{
			mProperties.SurfaceFormat.format = format;
		}

		void Set(std::vector<VkPresentModeKHR> &presentModePriorities)
		{
			mPresentModePriorities = presentModePriorities;
		}
		
		void Set(std::vector<VkSurfaceFormatKHR> &surfaceFormatPriorities)
		{
			mSurfaceFormatPriorities = surfaceFormatPriorities;
		}

		std::vector<VkImage> &Images()
		{
			return mImages;
		}

		VkFormat &Format()
		{
			return mProperties.SurfaceFormat.format;
		}

		VkImageUsageFlags &Usage()
		{
			return mProperties.ImageUsage;
		}

	public:
		DefineGetHandleFunc(VkSwapchainKHR)

	private:
		VulkanDevice &mDevice;
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		VkSwapchainKHR handle{ VK_NULL_HANDLE };

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
			VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
			VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
		};

		VkPresentModeKHR mPresentMode{};
		
		std::set<VkImageUsageFlagBits> mImageUsageFlags;
	};
}

