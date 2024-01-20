#pragma once

#include "Common.h"
#include "Graphics/Texture.h"
#include "Image.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{

class Texture : public SuperTexture, public Image
{
public:
    using Super = SuperTexture;
	VKCPP_SWAPPABLE(Texture)

public:
	Texture();

	Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type);

    Texture(Device *device, Image &&image, ImageView &&view);

    ~Texture();

    void Construct(Device *devcie, VkFormat format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, VkImageUsageFlags usage, VkSampleCountFlags sampleFlags = VK_SAMPLE_COUNT_1_BIT);

public:
    operator bool() const
    {
		return handle != VK_NULL_HANDLE;
    }

    VkDescriptorImageInfo GetDescriptorInfo() const
    {
		return VkDescriptorImageInfo{
            .sampler     = VK_NULL_HANDLE,
            .imageView   = view,
            .imageLayout = layout,
        };
    }

    uint32_t GetArrayLayers() const
	{
		return Image::GetArrayLayers();
	}

	uint32_t GetMipLevels() const
	{
		return Image::GetMipLevels();
	}

    const ImageView &GetImageView() const
    {
		return view;
    }

    VkImageLayout GetLayout() const
    {
		return layout;
    }

    void SetLayout(VkImageLayout value)
    {
		layout = value;
    }

    void Swap(Texture &other)
	{
		Image::Swap(other);
		std::swap(_width,       other._width      );
		std::swap(_height,      other._height     );
		std::swap(_mipLevels,   other._mipLevels  );
		std::swap(_arrayLayers, other._arrayLayers);
		std::swap( view,        other.view        );
		std::swap( layout,      other.layout      );
	}

protected:
    ImageView view;

    VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

}
}
