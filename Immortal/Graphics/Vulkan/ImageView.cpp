#include "ImageView.h"
#include "Image.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

inline VkImageViewType GetViewType(VkImageType type, VkImageCreateFlags flags, uint32_t arrayLayers)
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

    if (flags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
    {
		return arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
    }
    else if (type == VK_IMAGE_TYPE_2D)
    {
		viewType = arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (type == VK_IMAGE_TYPE_1D)
    {
		viewType = arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    }
    else if (type == VK_IMAGE_TYPE_3D)
    {
		viewType = VK_IMAGE_VIEW_TYPE_3D;
    }

    return viewType;
}

ImageView::ImageView() :
    Handle{},
    device{},
    image{}
{

}

ImageView::ImageView(Device *device, Image *image) :
    Handle{},
    device{ device },
    image{ image }
{
	Instantiate(GetViewType(image->GetType(), image->GetFlags(), image->GetArrayLayers()), 0, 0, image->GetMipLevels(), image->GetArrayLayers());
}

ImageView::ImageView(Image *image, uint32_t baseMipLevel, uint32_t baseArrayLayer):
    Handle{},
    device{ image->Get<Device>() },
    image{ image }
{
	Instantiate(GetViewType(image->GetType(), image->GetFlags(), image->GetArrayLayers()), baseMipLevel, baseArrayLayer, image->GetMipLevels(), image->GetArrayLayers());
}

ImageView::~ImageView()
{
    Release();
}

void ImageView::Release()
{
    if (!device && image)
    {
		device = image->Get<Device>();
    }

	if (device)
    {
        device->DestroyAsync(handle);
		handle = VK_NULL_HANDLE;
		image = nullptr;
    }
}

void ImageView::Instantiate( VkImageViewType viewType, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers)
{
    VkFormat format = image->GetFormat();

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
    viewInfo.image            = *image;
    viewInfo.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

    Check(device->Create(&viewInfo, &handle));
}

}
}
