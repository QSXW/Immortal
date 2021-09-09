#pragma once

#include "Common.h"
#include "Device.h"
#include "Image.h"

namespace Immortal
{
namespace Vulkan
{
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

        ImageView(ImageView &&other);

        ~ImageView();

    public:
        template <class T>
        T &Get()
        {
            if constexpr (typeof<T, VkImageView>())
            {
                return handle;
            }
        }

        void Set(Image *image)
        {
            this->image = image;
        }

    private:
        Device *device{ nullptr };

        Image *image{ nullptr };

        VkImageView handle{ VK_NULL_HANDLE };

        VkFormat format{};

        VkImageSubresourceRange subresourceRange{};
    };
}
}
