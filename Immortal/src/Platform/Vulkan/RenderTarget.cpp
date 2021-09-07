#include "impch.h"
#include "RenderTarget.h"

#include "Image.h"

namespace Immortal
{
namespace Vulkan
{
Unique<RenderTarget> RenderTarget::Create(Image &&image)
{
    auto &device = image.Get<Device>();
    VkFormat depthFormat = Vulkan::SuitableDepthFormat(device.Get<PhysicalDevice>().Handle());

    Image depthImage{ &device, image.Extent(), depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY };

    std::vector<Image> images;
    images.push_back(std::move(image));
    images.push_back(std::move(depthImage));

    return MakeUnique<RenderTarget>(std::move(images));
};

RenderTarget::RenderTarget(std::vector<Image> &&images) :
    device{ images.back().Get<Device>() },
    images{ std::move(images) }
{
    SLASSERT(!this->images.empty() && "Should specify at least 1 image");
    std::set<VkExtent2D, CompareExtent2D> uniqueExtent;

    auto ImageExtent = [](const Image &image) -> VkExtent2D
    {
        auto &extent = image.Extent();
        return { extent.width,  extent.height };
    };

    std::transform(this->images.begin(), this->images.end(), std::inserter(uniqueExtent, uniqueExtent.end()), ImageExtent);
    SLASSERT(uniqueExtent.size() == 1 && "Extent size is not unique");

    extent = *uniqueExtent.begin();

    for (auto &image : this->images)
    {
        if (image.Type() != VK_IMAGE_TYPE_2D)
        {
            LOG::ERR("Image type is not 2D");
        }

        views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
        attachments.emplace_back(Attachment{ image.Format(), image.SampleCount(), image.Usage() });
    }
}
}
}
