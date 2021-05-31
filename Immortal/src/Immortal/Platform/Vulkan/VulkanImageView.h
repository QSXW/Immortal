#pragma once

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace Immortal
{
	class VulkanImageView
	{
	public:
		VulkanImageView(VulkanImage     &image,
			            VkImageViewType viewType,
			            VkFormat        format         = VK_FORMAT_UNDEFINED,
			            UINT32          baseMipLevel   = 0,
			            UINT32          baseArrayLevel = 0,
			            UINT32          nMipLevels     = 0,
			            UINT32          nArrayLayers   = 0);

		~VulkanImageView();

	private:
		VulkanDevice &mDevice;

		VulkanImage *mImage{};

		VkImageView mHandle{ VK_NULL_HANDLE };

		VkFormat mFormat{};

		VkImageSubresourceRange mSubresourceRange{};
	};
}

