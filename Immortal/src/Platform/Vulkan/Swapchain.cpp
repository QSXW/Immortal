#include "impch.h"
#include "Swapchain.h"

#include "Framework/Utils.h"

namespace Immortal
{
namespace Vulkan
{
	inline UINT32 SelectImageCount(UINT32 request, UINT32 min, UINT32 max)
	{
		Utils::Clamp(request, min, max);
		return request;
	}

	inline UINT32 SelectImageArrayLayers(UINT32 request, UINT32 max)
	{
		Utils::Clamp(request, 1u, max);
		return request;
	}

	inline VkExtent2D SelectExtent(VkExtent2D request, const VkExtent2D &min, const VkExtent2D &max, const VkExtent2D &current)
	{
		if (request.width < 1 || request.height < 1)
		{
			IM_CORE_WARN(LOGB("(交换链) 2D图片尺寸 ({0}, {1}) 不支持. 已转为默认尺寸 ({2}, {3}).", "(Swapchain) Image extent ({0}, {1}) not supported. Selecting ({2}, {3})."),
				request.width, request.height, current.width, current.height);
			return current;
		}

		Utils::Clamp<UINT32>(request.width, min.width, max.width);
		Utils::Clamp<UINT32>(request.height, min.height, max.height);
		return request;
	}

	inline VkSurfaceFormatKHR SelectSurfaceFormat(const VkSurfaceFormatKHR request, const std::vector<VkSurfaceFormatKHR> &available, const std::vector<VkSurfaceFormatKHR> &priorities)
	{
		auto it = std::find_if(available.begin(), available.end(), [&request](const VkSurfaceFormatKHR &surface)
			{
				if (surface.format == request.format && surface.colorSpace == request.colorSpace)
				{
					return true;
				}
				return false;
			});

		// If the requested surface format isn't found, then try to request a format from the priority list
		if (it == available.end())
		{
			for (auto &f : priorities)
			{
				it = std::find_if(available.begin(), available.end(), [&f](const VkSurfaceFormatKHR &surface)
					{
						if (surface.format == f.format && surface.colorSpace == f.colorSpace)
						{
							return true;
						}
						return false;
					});
				if (it != available.end())
				{
				__IM__000_000_001_ReturnSurfaceFormat:
					IM_CORE_WARN(LOGB("(Swapchain) Surface 格式 ({0}) 不支持. 已选择 ({1}).", "(Swapchain) Surface format ({0}) not supported. Selecting ({1})."),
						Vulkan::Stringify(request), Vulkan::Stringify(*it));
					return *it;
				}
			}
			it = available.begin();
			goto __IM__000_000_001_ReturnSurfaceFormat;
		}
		else
		{
			IM_CORE_INFO(LOGB("(Swapchain) 选择 Surface 格式: {0}", "(Swapchain) Surface format selected: {0}"), Vulkan::Stringify(request));
		}
		return *it;
	}

	inline bool ValidateFormatFeature(VkImageUsageFlagBits imageUsage, VkFormatFeatureFlags supportedFeatures)
	{
		switch (imageUsage)
		{
		case VK_IMAGE_USAGE_STORAGE_BIT:
			return VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & supportedFeatures;
		default:
			return true;
		}
	}

	inline std::set<VkImageUsageFlagBits> SelectImageUsage(const std::set<VkImageUsageFlagBits> &request, VkImageUsageFlags support, VkFormatFeatureFlags supportedFeatures)
	{
		std::set<VkImageUsageFlagBits> validated;

		for (auto flag : request)
		{
			if ((flag & support) && ValidateFormatFeature(flag, supportedFeatures))
			{
				validated.insert(flag);
			}
			else
			{
				IM_CORE_WARN(LOGB("(Swapchain) 请求的Image usage ({0}) 不支持", "(Swapchain) Image usage ({0}) requested but not supported."), Vulkan::Stringify(flag));
			}
		}

		if (validated.empty())
		{
			static const VkImageUsageFlagBits imageUsageFlags[] = {
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_IMAGE_USAGE_STORAGE_BIT,
				VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT
			};

			for (size_t i = 0; i < ARRAY_LEN(imageUsageFlags); i++)
			{
				if ((imageUsageFlags[i] & support) && ValidateFormatFeature(imageUsageFlags[i], supportedFeatures))
				{
					validated.insert(imageUsageFlags[i]);
					break;
				}
			}
		}

		IM_CORE_ASSERT(!validated.empty(), LOGB("没有找到兼容的Image Usage", "No compatible image usage found."));
#if     IMMORTAL_CHECK_DEBUG
		IM_CORE_INFO(LOGB("(Swapchain)Image使用标识:", "(Swapchain) Image usage flags:"));
		for (auto &flag : validated)
		{
			IM_CORE_INFO("  \t{0}", Vulkan::Stringify(flag));
		}
#endif

		return validated;
	}

	inline VkImageUsageFlags CompositeImageFlags(const std::set<VkImageUsageFlagBits> &imageUsageFlags)
	{
		VkImageUsageFlags imageUsage{ 0 };
		for (auto flag : imageUsageFlags)
		{
			imageUsage |= flag;
		}
		return imageUsage;
	}

	inline VkSurfaceTransformFlagBitsKHR SelectTransform(VkSurfaceTransformFlagBitsKHR request, VkSurfaceTransformFlagsKHR support, VkSurfaceTransformFlagBitsKHR current)
	{
		if (request & support)
		{
			return request;
		}

		IM_CORE_WARN(LOGB("(Swapchain) Surface transform '{0}' 不支持，已转为'{1}'.", "(Swapchain) Surface transform '{0}' not supported. Selecting '{1}'."), Vulkan::Stringify(request), Vulkan::Stringify(current));

		return current;
	}

	inline VkCompositeAlphaFlagBitsKHR SelectCompositeAlpha(VkCompositeAlphaFlagBitsKHR request, VkCompositeAlphaFlagsKHR support)
	{
		if (request & support)
		{
			return request;
		}

		static const VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (size_t i = 0; i < ARRAY_LEN(compositeAlphaFlags); i++)
		{
			if (compositeAlphaFlags[i] & support)
			{
				IM_CORE_WARN(LOGB("(Swapchain) Composite alpha '{0}' 不支持. 已转为 '{1}.", "(Swapchain) Composite alpha '{0}' not supported. Selecting '{1}."),
					Vulkan::Stringify(request), Vulkan::Stringify(compositeAlphaFlags[i]));
				return compositeAlphaFlags[i];
			}
		}

		IM_CORE_ASSERT(false, LOGB("没有兼容的Composite Alpha", "No compatible composite alpha found."));
		return Utils::NullValue<VkCompositeAlphaFlagBitsKHR>();
	}

	inline VkPresentModeKHR SelectPresentMode(VkPresentModeKHR request, const std::vector<VkPresentModeKHR> &available, const std::vector<VkPresentModeKHR> &priorities)
	{
		auto it = std::find(available.begin(), available.end(), request);

		if (it == available.end())
		{
			// If nothing found, always default to FIFO
			VkPresentModeKHR chosen = VK_PRESENT_MODE_FIFO_KHR;

			for (auto &presentMode : priorities)
			{
				if (std::find(available.begin(), available.end(), presentMode) != available.end())
				{
					chosen = presentMode;
				}
			}

			IM_CORE_WARN(LOGB("(Swapchain) 显示模式 '{0}' 不支持. 已转为 '{}'.", "(Swapchain) Present mode '{}' not supported. Selecting '{}'."),
				Vulkan::Stringify(request), Vulkan::Stringify(chosen));
			return chosen;
		}
		else
		{
			IM_CORE_INFO(LOGB("(Swapchain) 选择显示模式: {0}", "(Swapchain) Present mode selected: {0}"), Vulkan::Stringify(request));
			return *it;
		}
	}

	Swapchain::Swapchain(Device &device,
		VkSurfaceKHR                         surface,
		const VkExtent2D &extent,
		const UINT32                         imageCount,
		const VkSurfaceTransformFlagBitsKHR  transform,
		const VkPresentModeKHR               presentMode,
		const std::set<VkImageUsageFlagBits> &imageUsageFlags) :
		Swapchain{ *this, device, surface, extent, imageCount, transform, presentMode, imageUsageFlags }
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
	Swapchain::Swapchain(Swapchain &oldSwapchain,
		Device &device,
		VkSurfaceKHR                         surface,
		const VkExtent2D &extent,
		const uint32_t                       imageCount,
		const VkSurfaceTransformFlagBitsKHR  transform,
		const VkPresentModeKHR               presentMode,
		const std::set<VkImageUsageFlagBits> &imageUsageFlags) :
		mDevice{ device },
		mSurface{ surface }
	{
		mPresentModePriorities = oldSwapchain.mPresentModePriorities;
		mSurfaceFormatPriorities = oldSwapchain.mSurfaceFormatPriorities;

		VkPhysicalDevice &physicalDeviceHandle = mDevice.GraphicsProcessingUnit().Handle();
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		Vulkan::Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceHandle, mSurface, &surfaceCapabilities));

		UINT32 surfaceFormatCount{ 0U };
		Vulkan::Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, mSurface, &surfaceFormatCount, nullptr));
		mSurfaceFormats.resize(surfaceFormatCount);
		Vulkan::Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, mSurface, &surfaceFormatCount, mSurfaceFormats.data()));

#if     IMMORTAL_CHECK_DEBUG
		IM_CORE_INFO(LOGB("Surface 支持下列格式:", "Surface supports the following surface formats:"));
		for (auto &f : mSurfaceFormats)
		{
			IM_CORE_INFO("  \t{0}", Vulkan::Stringify(f));
		}
#endif
		UINT32 presentModeCount{ 0U };
		Vulkan::Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, mSurface, &presentModeCount, nullptr));
		mPresentModes.resize(presentModeCount);
		Vulkan::Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, mSurface, &presentModeCount, mPresentModes.data()));

#if     IMMORTAL_CHECK_DEBUG
		IM_CORE_INFO(LOGB("Surface 支持下列显示模式:", "Surface supports the following present modes:"));
		for (auto &p : mPresentModes)
		{
			IM_CORE_INFO("  \t{0}", Vulkan::Stringify(p));
		}
#endif

		mProperties.ImageCount = SelectImageCount(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
		mProperties.Extent = SelectExtent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
		mProperties.ArrayLayers = SelectImageArrayLayers(1U, surfaceCapabilities.maxImageArrayLayers);
		mProperties.SurfaceFormat = SelectSurfaceFormat(mProperties.SurfaceFormat, mSurfaceFormats, mSurfaceFormatPriorities);

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDeviceHandle, mProperties.SurfaceFormat.format, &formatProperties);
		mImageUsageFlags = SelectImageUsage(imageUsageFlags, surfaceCapabilities.supportedUsageFlags, formatProperties.optimalTilingFeatures);
		mProperties.ImageUsage = CompositeImageFlags(mImageUsageFlags);
		mProperties.PreTransform = SelectTransform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
		mProperties.CompositeAlpha = SelectCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);
		mProperties.OldSwapchain = oldSwapchain.handle;
		mProperties.PresentMode = presentMode;
	}

	Swapchain::~Swapchain()
	{
		Vulkan::IfNotNullThen<VkSwapchainKHR>(vkDestroySwapchainKHR, mDevice.Handle(), handle);
	}

	void Swapchain::Create()
	{
		// Revalidate the present mode and surface format
		mProperties.PresentMode = SelectPresentMode(mProperties.PresentMode, mPresentModes, mPresentModePriorities);
		mProperties.SurfaceFormat = SelectSurfaceFormat(mProperties.SurfaceFormat, mSurfaceFormats, mSurfaceFormatPriorities);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.minImageCount = mProperties.ImageCount;
		createInfo.imageExtent = mProperties.Extent;
		createInfo.presentMode = mProperties.PresentMode;
		createInfo.imageFormat = mProperties.SurfaceFormat.format;
		createInfo.imageColorSpace = mProperties.SurfaceFormat.colorSpace;
		createInfo.imageArrayLayers = mProperties.ArrayLayers;
		createInfo.imageUsage = mProperties.ImageUsage;
		createInfo.preTransform = mProperties.PreTransform;
		createInfo.compositeAlpha = mProperties.CompositeAlpha;
		createInfo.oldSwapchain = mProperties.OldSwapchain;
		createInfo.surface = mSurface;
		createInfo.clipped = VK_TRUE; // Get the best performance by enabling clipping.

		auto &device = mDevice.Handle();
		Vulkan::Check(vkCreateSwapchainKHR(device, &createInfo, nullptr, &handle));

		UINT32 imageAvailable{ 0U };
		Vulkan::Check(vkGetSwapchainImagesKHR(device, handle, &imageAvailable, nullptr));

		mImages.resize(imageAvailable);
		Vulkan::Check(vkGetSwapchainImagesKHR(device, handle, &imageAvailable, mImages.data()));
	}
}
}
