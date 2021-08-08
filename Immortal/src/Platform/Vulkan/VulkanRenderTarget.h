#pragma once

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"

namespace Immortal
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
			return !(lhs.width == rhs.width && lhs.height == rhs.height) && (lhs.width < rhs.width && lhs.height < rhs.height);
		}
	};

	class VulkanRenderTarget
	{
	public:
		using CreateFunc = std::function<Unique<VulkanRenderTarget>(VulkanImage &&swapchainImage)>;
		static const CreateFunc DefaultCreateFunc;

		VulkanRenderTarget(std::vector<VulkanImage> &&images);

	private:
		VulkanDevice &mDevice;

		VkExtent2D mExtent{};

		std::vector<VulkanImage> mImages;

		std::vector<Attachment> mAttachments;

		std::vector<VulkanImageView> mViews;

		std::vector<UINT32> mInputAttachments{};

		std::vector<UINT32> mOutputAttachments{ 0 };
	};
}

