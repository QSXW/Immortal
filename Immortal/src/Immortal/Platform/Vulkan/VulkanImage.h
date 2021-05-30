#pragma once

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace Immortal
{
	class VulkanImage
	{
	public:
		VulkanImage(VulkanDevice & device, VkImage handle, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags imageUsage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

	private:
		VulkanDevice &mDevice;

		VkImage mHandle{ VK_NULL_HANDLE };

		VmaAllocation mMemory{ VK_NULL_HANDLE };

		VkImageType mType{};

		VkExtent3D mExtent;

		VkFormat mFormat{};

		VkImageUsageFlags mUsage{};

		VkSampleCountFlagBits mSampleCount{};

		VkImageTiling mTiling{};

		VkImageSubresource mSubresource{};

		UINT32 mArrayLayerCount{ 0 };

	};

}
