#pragma once

#include "vkcommon.h"
#include "Device.h"
#include "Image.h"

namespace Immortal
{
namespace Vulkan
{
	class ImageView
	{
	public:
		ImageView(Image&          image,
			      VkImageViewType viewType,
			      VkFormat        format         = VK_FORMAT_UNDEFINED,
			      UINT32          baseMipLevel   = 0,
			      UINT32          baseArrayLevel = 0,
			      UINT32          nMipLevels     = 0,
			      UINT32          nArrayLayers   = 0);

		ImageView(ImageView&& other);

		~ImageView();

		// @inline
	public:
		void Set(Image* image)
		{
			mImage = image;
		}

	private:
		Device &device;

		Image* mImage{};

		VkImageView handle{ VK_NULL_HANDLE };

		VkFormat mFormat{};

		VkImageSubresourceRange mSubresourceRange{};
	};
}
}
