#pragma once

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

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
    void INIT(VkImage image, VkImageViewType viewType, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers)
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

        Check(vkCreateImageView(device->Get<VkDevice>(), &viewInfo, nullptr, &handle));
    }

private:
    VkImageView handle{ VK_NULL_HANDLE };

    Device *device{ nullptr };

    Image *refImage{ nullptr };

    VkFormat format{};

    VkImageSubresourceRange subresourceRange{};
};
}
}
