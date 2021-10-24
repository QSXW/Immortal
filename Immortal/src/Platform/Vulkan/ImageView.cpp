#include "impch.h"
#include "ImageView.h"
#include "image.h"

namespace Immortal
{
namespace Vulkan
{

ImageView::ImageView(Image *image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
    device{ image->Get<Device *>() },
    refImage{ image },
    format{ format }
{
    if (format == VK_FORMAT_UNDEFINED)
    {
        this->format = format = image->Format();
    }

    INIT(
        image->Handle(),
        viewType,
        baseMipLevel,
        baseArrayLevel,
        nMipLevels == 0 ? image->Subresource().mipLevel : nMipLevels,
        nArrayLayers == 0 ? image->Subresource().arrayLayer : nArrayLayers
        );

    refImage->Views().emplace(this);
}

ImageView::ImageView(Device *device, VkImage image, VkImageViewType viewType, VkFormat format, UINT32 baseMipLevel, UINT32 baseArrayLevel, UINT32 nMipLevels, UINT32 nArrayLayers) :
    device{ device },
    format{ format }
{
    INIT(image, viewType, baseMipLevel, baseArrayLevel, nMipLevels, nArrayLayers);
}

ImageView::ImageView(ImageView&& other) :
    device{ other.device },
    refImage{ other.refImage },
    handle{ other.handle },
    format{ other.format },
    subresourceRange{ other.subresourceRange }
{
    auto &views = refImage->Views();
    views.erase(&other);
    views.emplace(this);

    other.handle = VK_NULL_HANDLE;
}

ImageView::~ImageView()
{
    device->Destory(handle);
}

}
}
