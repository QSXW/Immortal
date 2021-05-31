#include "impch.h"
#include "VulkanRenderTarget.h"

#include "VulkanImage.h"

namespace Immortal
{
	const VulkanRenderTarget::CreateFunc VulkanRenderTarget::DefaultCreateFunc = [](VulkanImage &&swapchainImage) -> Unique<VulkanRenderTarget>
	{
		auto &device = swapchainImage.Device();
		VkFormat depthFormat = Vulkan::SuitableDepthFormat(device.GraphicsProcessingUnit().Handle());

		VulkanImage depthImage{ device, swapchainImage.Extent(), depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY };

		std::vector<VulkanImage> images;
		images.push_back(std::move(swapchainImage));
		images.push_back(std::move(depthImage));

		return MakeUnique<VulkanRenderTarget>(std::move(images));
	};


	VulkanRenderTarget::VulkanRenderTarget(std::vector<VulkanImage>&& images) :
		mDevice{ images.back().Device() },
		mImages{ std::move(images) }
	{
		IM_CORE_ASSERT(!mImages.empty(), LOGB("std::vector<VulkanImage> 至少有一个VulkanImage才能创建RenderTarget", "Should specify at least 1 image"));
		std::set<VkExtent2D, CompareExtent2D> uniqueExtent;

		auto ImageExtent = [](const VulkanImage &image) -> VkExtent2D
		{
			auto extent = image.Extent();
			return { extent.width,  extent.height };
		};

		std::transform(mImages.begin(), mImages.end(), std::inserter(uniqueExtent, uniqueExtent.end()), ImageExtent);
		IM_CORE_ASSERT(uniqueExtent.size() == 1, LOGB("Extent的大小不唯一", "Extent size is not unique"));

		mExtent = *uniqueExtent.begin();

		for (auto &image : mImages)
		{
			if (image.Type() != VK_IMAGE_TYPE_2D)
			{
				IM_CORE_ERROR(LOGB("图片类型不是2D", "Image type is not 2D"));
			}

			mViews.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
			mAttachments.emplace_back(Attachment{ image.Format(), image.SampleCount(), image.Usage() });
		}
	}

}