#include "ImageView.h"
#include "Image.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

inline VkImageViewType GetViewType(VkImageType type, uint32_t arrayLayers)
{
    VkImageViewType ret = VK_IMAGE_VIEW_TYPE_2D;

    if (type == VK_IMAGE_TYPE_2D)
    {
        ret = arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (type == VK_IMAGE_TYPE_1D)
    {
        ret = arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    }
    else if (type == VK_IMAGE_TYPE_3D)
    {
        ret = VK_IMAGE_VIEW_TYPE_3D;
    }

    return ret;
}

ImageView::ImageView() :
    device{},
    handle{},
    format{}
{

}

ImageView::ImageView(Image *image, VkImageViewType type, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t mipLevels, uint32_t arrayLayers) :
    device{ image->GetAddress<Device>() },
    format{ image->GetFormat() }
{
    auto &props = image->GetProperties();
    Instantiate(*image, type, baseMipLevel, baseArrayLayer,!mipLevels ? props.mipLevels : mipLevels, !arrayLayers ? props.arrayLayers : arrayLayers);
}

ImageView::ImageView(Image *image, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t mipLevels, uint32_t arrayLayers) :
    ImageView{ image, GetViewType(image->GetType(), arrayLayers ? arrayLayers : image->GetArrayLayer()), baseMipLevel, baseArrayLayer , mipLevels, arrayLayers }
{ 

}

ImageView::ImageView(Device *device, VkImage image, VkImageViewType viewType, VkFormat format, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers) :
    device{ device },
    format{ format }
{
    Instantiate(image, viewType, baseMipLevel, baseArrayLevel, mipLevels, arrayLayers);
}

ImageView::ImageView(ImageView &&other) :
    ImageView{}
{
    other.Swap(*this);
}

ImageView &ImageView::operator =(ImageView &&other)
{
    ImageView(std::move(other)).Swap(*this);
    return *this;
}

void ImageView::Swap(ImageView &other)
{
    std::swap(device, other.device);
    std::swap(handle, other.handle);
}

ImageView::~ImageView()
{
    if (device)
    {
        device->DestroyAsync(handle);
    }
}

void ImageView::Instantiate(VkImage image, VkImageViewType viewType, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers)
{
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.baseMipLevel   = baseMipLevel;
    subresourceRange.baseArrayLayer = baseArrayLevel;
    subresourceRange.levelCount     = mipLevels;
    subresourceRange.layerCount     = arrayLayers;
    subresourceRange.aspectMask     = IsDepthOnlyFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType         = viewType;
    viewInfo.format           = format;
    viewInfo.subresourceRange = subresourceRange;
    viewInfo.image            = image;
    viewInfo.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

    Check(device->Create(&viewInfo, &handle));
}

}
}
