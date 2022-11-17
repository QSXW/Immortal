#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Image;
class ImageView
{
public:
    using Primitive = VkImageView;
    VKCPP_OPERATOR_HANDLE()

public:
    ImageView();

    ImageView(Image *image, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0, uint32_t mipLevels = 0, uint32_t arrayLayers = 0);

    ImageView(Image *image, VkImageViewType type, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0, uint32_t mipLevels = 0, uint32_t arrayLayers = 0);

    ImageView(Device *device, VkImage image, VkImageViewType viewType, VkFormat format, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers);

    ImageView(ImageView &&other);

    ImageView &operator =(ImageView &&other);

    void Swap(ImageView &other);

    ~ImageView();

    void Instantiate(VkImage image, VkImageViewType viewType, uint32_t baseMipLevel, uint32_t baseArrayLevel, uint32_t mipLevels, uint32_t arrayLayers);

    ImageView(const ImageView &other) = delete;
    ImageView &operator=(const ImageView &other) = delete;

protected:
    Device *device;

    VkFormat format;
};
}
}
