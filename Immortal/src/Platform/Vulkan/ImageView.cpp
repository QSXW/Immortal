#include "impch.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan {
    ImageView::ImageView(Image &image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
        device{ image.Get<Device>() },
        image{ &image },
        format{ format }
    {
        if (format == VK_FORMAT_UNDEFINED)
        {
            format = format = image.Format();
        }

        subresourceRange.baseMipLevel   = baseMipLevel;
        subresourceRange.baseArrayLayer = baseArrayLevel;
        subresourceRange.levelCount     = nMipLevels == 0 ? image.Subresource().mipLevel : nMipLevels;
        subresourceRange.layerCount     = nArrayLayers == 0 ? image.Subresource().arrayLayer : nArrayLayers;
        subresourceRange.aspectMask     = Vulkan::IsDepthOnlyFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType         = viewType;
        viewInfo.format           = format;
        viewInfo.subresourceRange = subresourceRange;
        viewInfo.image            = image.Handle();

        Vulkan::Check(vkCreateImageView(device.Get<VkDevice>(), &viewInfo, nullptr, &handle));
        this->image->Views().emplace(this);
    }

    ImageView::ImageView(ImageView&& other) :
        device{ other.device },
        image{ other.image },
        handle{ other.handle },
        format{ other.format },
        subresourceRange{ other.subresourceRange }
    {
        auto& views = image->Views();
        views.erase(&other);
        views.emplace(this);

        other.handle = VK_NULL_HANDLE;
    }

    ImageView::~ImageView()
    {
        Vulkan::IfNotNullThen<VkImageView>(vkDestroyImageView, device.Get<VkDevice>(), handle);
    }
}
}
