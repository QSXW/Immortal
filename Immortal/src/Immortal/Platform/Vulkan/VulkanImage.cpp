#include "impch.h"
#include "VulkanImage.h"

namespace Immortal
{
namespace Vulkan
{
	inline VkImageType ImageType(VkExtent3D extent)
	{
		VkImageType result{};

		uint32_t dimension{ 0 };

		if (extent.width >= 1)
		{
			dimension++;
		}

		if (extent.height >= 1)
		{
			dimension++;
		}

		if (extent.depth > 1)
		{
			dimension++;
		}

		switch (dimension)
		{
			case 1:
				result = VK_IMAGE_TYPE_1D;
				break;
			case 2:
				result = VK_IMAGE_TYPE_2D;
				break;
			case 3:
				result = VK_IMAGE_TYPE_3D;
				break;
			default:
				IM_CORE_ERROR(LOGB("Õº∆¨∏Ò Ω¥ÌŒÛ", "No image type found."));
				break;
		}

		return result;
	}
}
	

	VulkanImage::VulkanImage(VulkanDevice &device, VkImage handle, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VkSampleCountFlagBits sampleCount) :
		mDevice{ device },
		mHandle{ handle },
		mType{ Vulkan::ImageType(extent) },
		mExtent{ extent },
		mFormat{ format },
		mSampleCount{ sampleCount },
		mUsage{ imageUsage }
	{
		mSubresource.mipLevel   = 1;
		mSubresource.arrayLayer = 1;
	}
}