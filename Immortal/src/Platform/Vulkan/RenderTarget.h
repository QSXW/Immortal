#pragma once

#include "vkcommon.h"
#include "Device.h"
#include "Image.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{
	/**
	* @brief Description of render pass attachments.
	* Attachment descriptions can be used to automatically create render target images.
	*/
	struct Attachment
	{
		VkFormat format{ VK_FORMAT_UNDEFINED };

		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };

		VkImageUsageFlags usage{ VK_IMAGE_USAGE_SAMPLED_BIT };

		VkImageLayout initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		Attachment() = default;

		Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) :
			format{ format },
			samples{ samples },
			usage{ usage }
		{

		}
	};

	struct CompareExtent2D
	{
		bool operator()(const VkExtent2D &lhs, const VkExtent2D &rhs) const
		{
			return !(lhs.width == rhs.width && lhs.height == rhs.height) && (lhs.width < rhs.width &&lhs.height < rhs.height);
		}
	};

	class RenderTarget
	{
	public:
		using CreateFunc = std::function<Unique<RenderTarget>(Image &&swapchainImage)>;
		static const CreateFunc DefaultCreateFunc;

		RenderTarget(std::vector<Image> &&images);

	private:
		Device &device;

		VkExtent2D mExtent{};

		std::vector<Image> mImages;

		std::vector<Attachment> mAttachments;

		std::vector<ImageView> mViews;

		std::vector<UINT32> mInputAttachments{};

		std::vector<UINT32> mOutputAttachments{ 0 };
	};
}
}
