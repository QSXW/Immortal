#pragma once

#include "Common.h"
#include "Handle.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Image;
class ImageView : public Handle<VkImageView>
{
public:
    VKCPP_SWAPPABLE(ImageView)

public:
    ImageView();

    ImageView(Device *device, Image *image);

    ImageView(Image *image, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0);

    /** Single mip range / layer range (e.g. one array slice as a 2D view). */
    ImageView(Image *image, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t mipLevelCount, uint32_t arrayLayerCount);

    ~ImageView();

    void Release();

    void Instantiate(VkImageViewType viewType, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers);

public:
    void Swap(ImageView &other)
    {
        Handle::Swap(other);
		std::swap(device, other.device);
        std::swap(image,  other.image);
    }

protected:
    Device *device;

    Image *image;
};
}
}
