#include "impch.h"
#include "VulkanImageView.h"

namespace Immortal
{
	VulkanImageView::VulkanImageView(VulkanImage &image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
		mDevice{ image.Device() },
		mImage{ &image },
		mFormat{ format }
	{
		if (format == VK_FORMAT_UNDEFINED)
		{
			mFormat = format = image.Format();
		}

		mSubresourceRange.baseMipLevel   = baseMipLevel;
		mSubresourceRange.baseArrayLayer = baseArrayLevel;
		mSubresourceRange.levelCount     = nMipLevels == 0 ? image.Subresource().mipLevel : nMipLevels;
		mSubresourceRange.layerCount     = nArrayLayers == 0 ? image.Subresource().arrayLayer : nArrayLayers;
		mSubresourceRange.aspectMask     = Vulkan::IsDepthOnlyFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType         = viewType;
		viewInfo.format           = format;
		viewInfo.subresourceRange = mSubresourceRange;
		viewInfo.image            = image.Handle();

		Vulkan::Check(vkCreateImageView(mDevice.Handle(), &viewInfo, nullptr, &mHandle));
		mImage->Views().emplace(this);
	}

	VulkanImageView::VulkanImageView(VulkanImageView && other) :
		mDevice{ other.mDevice },
		mImage{ other.mImage },
		mHandle{ other.mHandle },
		mFormat{ other.mFormat },
		mSubresourceRange{ other.mSubresourceRange }
	{
		auto &views = mImage->Views();
		views.erase(&other);
		views.emplace(this);	

		other.mHandle = VK_NULL_HANDLE;
	}

	VulkanImageView::~VulkanImageView()
	{
		Vulkan::IfNotNullThen<VkImageView, &vkDestroyImageView>(mDevice.Handle(), mHandle);
	}
}