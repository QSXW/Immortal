#include "Texture.h"

#include "Device.h"

#include <algorithm>

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
	SetMeta(format, width, height, mipLevels, arrayLayers);
	Construct(device, format, width, height, mipLevels, arrayLayers, CAST(type));
}

Texture::Texture(Device *device, Image &&image, ImageView &&/*view*/) :
    Image{ std::move(image) },
    view{}
{
	view = ImageView{ device, GetImage() };
}

Texture::~Texture()
{
	Device *d = Get<Device>();
	for (VkImageView v : storageMipViews)
	{
		if (v && d)
		{
			d->DestroyAsync(v);
		}
	}
	storageMipViews.clear();
    view.Release();
	Image::Release();
}

void Texture::SetName(const char *name)
{
	device->SetName(VK_OBJECT_TYPE_IMAGE, (uint64_t)handle, name);
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

VkDescriptorImageInfo Texture::GetStorageDescriptorInfo(uint32_t mipLevel) const
{
	const uint32_t levels = GetMipLevels();
	const uint32_t mip = levels > 0 ? (std::min)(mipLevel, levels - 1u) : 0u;

	if (storageMipViews.size() < levels)
	{
		storageMipViews.resize(levels, VK_NULL_HANDLE);
	}

	if (storageMipViews[mip] == VK_NULL_HANDLE)
	{
		Device *dev = const_cast<Texture *>(this)->Get<Device>();
		if (!dev || !handle)
		{
			return VkDescriptorImageInfo{ VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
		}

		const uint32_t layers = GetArrayLayers();

		/* RWTexture2DArray / SPIR-V storage is always a 2D array image, not cube—even for 6-face cube-compatible images. */
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		if (GetType() == VK_IMAGE_TYPE_2D && layers > 1u)
		{
			viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
		else if (GetType() == VK_IMAGE_TYPE_1D)
		{
			viewType = layers > 1u ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
		}
		else if (GetType() == VK_IMAGE_TYPE_3D)
		{
			viewType = VK_IMAGE_VIEW_TYPE_3D;
		}

		VkImageSubresourceRange range{
			.aspectMask = GetAspectMask(),
			.baseMipLevel = mip,
			.levelCount = 1u,
			.baseArrayLayer = 0u,
			.layerCount = layers,
		};

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = {},
			.image = handle,
			.viewType = viewType,
			.format = GetFormat(),
			.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			.subresourceRange = range,
		};

		Check(dev->Create(&viewInfo, &storageMipViews[mip]));
	}

	return VkDescriptorImageInfo{
		.sampler = VK_NULL_HANDLE,
		.imageView = storageMipViews[mip],
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
}

}
}
