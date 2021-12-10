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
    ImageView(Image          *image,
              VkImageViewType viewType,
              VkFormat        format         = VK_FORMAT_UNDEFINED,
              UINT32          baseMipLevel   = 0,
              UINT32          baseArrayLevel = 0,
              UINT32          nMipLevels     = 0,
              UINT32          nArrayLayers   = 0);

    ImageView(Device          *device,
              VkImage          image,
              VkImageViewType  viewType,
              VkFormat         format,
              UINT32           baseMipLevel,
              UINT32           baseArrayLevel,
              UINT32           nMipLevels,
              UINT32           nArrayLayers
              );

    ImageView(ImageView &&other);

    ~ImageView();

    VkImageView &Handle()
    {
        return handle;
    }

    operator VkImageView&()
    {
        return handle;
    }

    template <class T>
    T Get()
    {
        if constexpr (is_same<T, VkImageView, VkImageView &>)
        {
            return handle;
        }
    }

    void Set(Image *image)
    {
        this->refImage = image;
    }

private:
    void INIT(VkImage image, VkImageViewType viewType, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers);

private:
    VkImageView handle{ VK_NULL_HANDLE };

    Device *device{ nullptr };

    Image *refImage{ nullptr };

    VkFormat format{};

    VkImageSubresourceRange subresourceRange{};
};
}
}
