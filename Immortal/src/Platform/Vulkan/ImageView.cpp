#include "impch.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan {
	ImageView::ImageView(Image& image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
		device{ image.Get<Device>() },
		mImage{ &image },
		mFormat{ format }
	{
		if (format == VK_FORMAT_UNDEFINED)
		{
			mFormat = format = image.Format();
		}

		mSubresourceRange.baseMipLevel = baseMipLevel;
		mSubresourceRange.baseArrayLayer = baseArrayLevel;
		mSubresourceRange.levelCount = nMipLevels == 0 ? image.Subresource().mipLevel : nMipLevels;
		mSubresourceRange.layerCount = nArrayLayers == 0 ? image.Subresource().arrayLayer : nArrayLayers;
		mSubresourceRange.aspectMask = Vulkan::IsDepthOnlyFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.subresourceRange = mSubresourceRange;
		viewInfo.image = image.Handle();

		Vulkan::Check(vkCreateImageView(device.Handle(), &viewInfo, nullptr, &handle));
		mImage->Views().emplace(this);
	}

	ImageView::ImageView(ImageView&& other) :
		device{ other.device },
		mImage{ other.mImage },
		handle{ other.handle },
		mFormat{ other.mFormat },
		mSubresourceRange{ other.mSubresourceRange }
	{
		auto& views = mImage->Views();
		views.erase(&other);
		views.emplace(this);

		other.handle = VK_NULL_HANDLE;
	}

	ImageView::~ImageView()
	{
		Vulkan::IfNotNullThen<VkImageView>(vkDestroyImageView, device.Handle(), handle);
	}
}
}