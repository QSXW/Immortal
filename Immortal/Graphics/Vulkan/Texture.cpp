#include "Texture.h"

#include "Device.h"
#include "Barrier.h"

namespace Immortal
{
namespace Vulkan
{

VkImageUsageFlags CAST(TextureType type)
{
	VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (type & TextureType::TransferSource)
    {
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (type & TextureType::TransferDestination)
    {
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (type & TextureType::ColorAttachment)
    {
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (type & TextureType::DepthStencilAttachment)
    {
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (type & TextureType::Storage)
    {
		flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    return flags;
}

Texture::Texture() :
    SuperTexture{},
    Image{},
    view{}
{

}

Texture::Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) :
    SuperTexture{},
    Image{},
    view{}
{
	SetMeta(width, height, mipLevels, arrayLayers);
	Construct(device, format, width, height, mipLevels, arrayLayers, CAST(type));
}

Texture::Texture(Device *device, Image &&image, ImageView && /*view*/) :
    Image{ std::move(image) },
    view{ std::move(view) }
{
	view = ImageView{ device, GetImage() };
}

Texture::~Texture()
{
    view.Release();
	Image::Release();
}

void Texture::Construct(Device *device, VkFormat format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, VkImageUsageFlags usage, VkSampleCountFlags sampleFlags)
{
    layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo imageCreateInfo{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext         = nullptr,
	    .flags         = arrayLayers == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : VkImageCreateFlags{},
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = format,
        .extent        = { width, height, 1 },
        .mipLevels     = mipLevels,
        .arrayLayers   = arrayLayers,
        .samples       = VkSampleCountFlagBits(sampleFlags),
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = usage,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (mipLevels > 1)
    {
		imageCreateInfo.usage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    }

    Image{
        device,
        imageCreateInfo,
        VMA_MEMORY_USAGE_GPU_ONLY
        }.Swap(*this);

    view = ImageView{ GetImage() };
}

}
}
