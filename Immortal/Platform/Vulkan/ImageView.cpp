#include "impch.h"
#include "ImageView.h"
#include "image.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

ImageView::ImageView(Image *image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
    device{ image->GetAddress<Device>() },
    refImage{ image },
    format{ format }
{
    if (format == VK_FORMAT_UNDEFINED)
    {
        this->format = format = image->Format();
    }

    Setup(
        image->Handle(),
        viewType,
        baseMipLevel,
        baseArrayLevel,
        nMipLevels == 0 ? image->Subresource().mipLevel : nMipLevels,
        nArrayLayers == 0 ? image->Subresource().arrayLayer : nArrayLayers
        );

    refImage->Views().emplace(this);
}

ImageView::ImageView(Device *device, VkImage image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
    device{ device },
    format{ format }
{
    Setup(image, viewType, baseMipLevel, baseArrayLevel, nMipLevels, nArrayLayers);
}

ImageView::ImageView(ImageView&& other) :
    device{ other.device },
    refImage{ other.refImage },
    handle{ other.handle },
    format{ other.format },
    subresourceRange{ other.subresourceRange }
{
    auto &views = refImage->Views();
    views.erase(&other);
    views.emplace(this);

    other.handle = VK_NULL_HANDLE;
}

ImageView::~ImageView()
{
    if (device)
    {
        device->Destroy(handle);
    }
}

void ImageView::Setup(VkImage image, VkImageViewType viewType, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers)
{
    subresourceRange.baseMipLevel   = baseMipLevel;
    subresourceRange.baseArrayLayer = baseArrayLevel;
    subresourceRange.levelCount     = nMipLevels;
    subresourceRange.layerCount     = nArrayLayers;
    subresourceRange.aspectMask     = IsDepthOnlyFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType         = viewType;
    viewInfo.format           = format;
    viewInfo.subresourceRange = subresourceRange;
    viewInfo.image            = image;
    viewInfo.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    Check(vkCreateImageView(*device, &viewInfo, nullptr, &handle));
}

}
}
