#pragma once

#include "vkcommon.h"
#include "Device.h"
#include "Image.h"

namespace Immortal
{
namespace Vulkan
{
    class ImageView
    {
    public:
        ImageView(Image&          image,
                  VkImageViewType viewType,
                  VkFormat        format         = VK_FORMAT_UNDEFINED,
                  UINT32          baseMipLevel   = 0,
                  UINT32          baseArrayLevel = 0,
                  UINT32          nMipLevels     = 0,
                  UINT32          nArrayLayers   = 0);

        ImageView(ImageView&& other);

        ~ImageView();
    public:
        void Set(Image* image)
        {
            this->image = image;
        }

    private:
        Device &device;

        Image* image{};

        VkImageView handle{ VK_NULL_HANDLE };

        VkFormat format{};

        VkImageSubresourceRange subresourceRange{};
    };
}
}
