#include "RenderTarget.h"

#include "Image.h"
#include "Barrier.h"
#include "RenderContext.h"
#include "DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

URef<RenderTarget> RenderTarget::Create(std::unique_ptr<Image> &&colorImage)
{
    std::vector<std::unique_ptr<Image>> images;

    auto device = colorImage->GetAddress<Device>();

    VkFormat depthFormat = SuitableDepthFormat(device->Get<PhysicalDevice>());

    auto depthImage = std::make_unique<Image>(
        device,
        colorImage->Extent(),
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );

    images.emplace_back(std::move(colorImage));
    images.emplace_back(std::move(depthImage));

    return new RenderTarget{std::move(images)};
};

RenderTarget::RenderTarget(Device *device, const RenderTarget::Description &description) :
    Super{ description },
    device{ device },
    sampler{ device, description.Attachments[0] }
{
    Create();
}

RenderTarget::RenderTarget(std::vector<std::unique_ptr<Image>> images) :
    device{ images.back()->GetAddress<Device>() }
{
    SLASSERT(!images.empty() && "Should specify at least 1 image");
    std::set<VkExtent2D, CompareExtent2D> uniqueExtent;

    auto ImageExtent = [](std::unique_ptr<Image> &image) -> VkExtent2D
    {
        auto &extent = image->Extent();
        return { extent.width,  extent.height };
    };

    std::transform(images.begin(), images.end(), std::inserter(uniqueExtent, uniqueExtent.end()), ImageExtent);
    SLASSERT(uniqueExtent.size() == 1 && "Extent size is not unique");

    auto &extent = *uniqueExtent.begin();

    desc.Width  = extent.width;
	desc.Height = extent.height;

    for (auto &image : images)
    {
        if (image->Type() != VK_IMAGE_TYPE_2D)
        {
            LOG::ERR("Image type is not 2D");
        }

        Attachment attachment{
            std::move(image),
            std::make_unique<ImageView>(image.get(), VK_IMAGE_VIEW_TYPE_2D)
        };

        if (attachment.image->IsDepth())
        {
            attachments.depth = std::move(attachment);
        }
        else
        {
            attachments.colors.emplace_back(std::move(attachment));
        }
    }
}

RenderTarget::RenderTarget(RenderTarget &&other) :
    device{ other.device }
{
    desc               = other.desc;
    attachments.colors = std::move(other.attachments.colors);
    attachments.depth  = std::move(other.attachments.depth);

    other.device = nullptr;
}

RenderTarget::~RenderTarget()
{

}

RenderTarget &RenderTarget::operator=(RenderTarget &&other)
{
    SLASSERT(&other == this && "You are trying to self-assigments, which is not acceptable.");
    device             = other.device;
    desc               = other.desc;
    attachments.colors = std::move(other.attachments.colors);
    attachments.depth  = std::move(other.attachments.depth);

    other.device       = nullptr;

    return *this;
}

void RenderTarget::Create()
{
    VkExtent3D extent = { desc.Width, desc.Height, 1 };
    VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    attachments.colors.clear();
    stagingImages.clear();
    size_t index = 0;
    for (auto &attachment : desc.Attachments)
    {
        if (attachment.IsDepth())
        {
            VkClearDepthStencilValue *depthClearValue = reinterpret_cast<VkClearDepthStencilValue *>(&clearValues[index]);
            depthClearValue->depth   = 1.0f;
            depthClearValue->stencil = 0.0f;
            depthFormat = attachment.format;
            attachments.depth.image.reset(new Image{
                device,
                extent,
                depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY
                });
            attachments.depth.view.reset(new ImageView{ attachments.depth.image.get(), VK_IMAGE_VIEW_TYPE_2D });
        }
        else
        {
            VkImageUsageFlags flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            attachments.colors.resize(attachments.colors.size() + 1);
            auto &color = attachments.colors.back();
            colorFormat = attachment.format;
            color.image.reset(new Image{
                device,
                extent,
                colorFormat,
                flags,
                VMA_MEMORY_USAGE_GPU_ONLY,
                });
            color.view.reset(new ImageView{ color.image.get(), VK_IMAGE_VIEW_TYPE_2D });

            auto image = new Image{
                device,
                extent,
                colorFormat,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_GPU_TO_CPU,
                VK_SAMPLE_COUNT_1_BIT,
                1, 1,
                VK_IMAGE_TILING_LINEAR
                };
            uint8_t *dataMapped = nullptr;
            image->Map((void **)&dataMapped);
            memset(dataMapped, -1, attachment.format.ComponentCount() *extent.width * extent.height * extent.depth);
            image->Unmap();
            stagingImages.emplace_back(image);
        }
        index++;
    }

    std::vector<VkAttachmentDescription> attchmentDescriptions{};
    std::vector<VkAttachmentReference> colorRefs{};

    for (auto &c : attachments.colors)
    {
        VkAttachmentDescription desc{};
        desc.format         = c.image->Format();
        desc.samples        = VK_SAMPLE_COUNT_1_BIT;
        desc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        desc.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attchmentDescriptions.emplace_back(std::move(desc));

        VkAttachmentReference colorRef{};
        colorRef.attachment = attchmentDescriptions.size() - 1;
        colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRefs.emplace_back(std::move(colorRef));
    }

    VkAttachmentDescription depthAttachmentDesc{};
    depthAttachmentDesc.format         = depthFormat;
    depthAttachmentDesc.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDesc.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attchmentDescriptions.emplace_back(std::move(depthAttachmentDesc));

    VkAttachmentReference depthRef{};
    depthRef.attachment = attchmentDescriptions.size() - 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount    = U32(colorRefs.size());
    subpassDescription.pColorAttachments       = colorRefs.data();
    subpassDescription.pDepthStencilAttachment = &depthRef;

    std::array<VkSubpassDependency, 2> dependencies{};

    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = U32(attchmentDescriptions.size());
    renderPassInfo.pAttachments    = attchmentDescriptions.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDescription;
    renderPassInfo.dependencyCount = U32(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();

    renderPass = new RenderPass(device, &renderPassInfo);

    SetupFramebuffer();
    SetupDescriptor();
}

void RenderTarget::SetupDescriptor()
{
    if (!descriptor)
    {
       descriptor = new ImageDescriptor{};
    }
    descriptor->Update(sampler, *attachments.colors[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    descriptorSet = new DescriptorSet{ device, RenderContext::DescriptorSetLayout };
    descriptorSet->Update(*descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

RenderTarget::operator uint64_t() const
{
    return *descriptorSet;
}

void RenderTarget::Resize(uint32_t x, uint32_t y)
{
    desc.Width  = x;
    desc.Height = y;

    Create();
}

uint64_t RenderTarget::PickPixel(uint32_t index, uint32_t x, uint32_t y, Format format)
{
    THROWIF(index >= attachments.colors.size(), SError::OutOfBound);

    uint64_t pixel = 0;

    uint8_t *ptr   = nullptr;
    Image *src = attachments.colors[index].image.get();
    Image *dst = stagingImages[index].get();

    VkImageSubresourceLayers subresourceLayers{};
    subresourceLayers.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceLayers.layerCount     = 1;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.mipLevel       = 0;

    VkImageCopy region{};
    region.extent         = src->Extent();
    region.srcOffset      = VkOffset3D{ 0, 0, 0 };
    region.srcSubresource = subresourceLayers;
    region.dstOffset      = VkOffset3D{ 0, 0, 0 };
    region.dstSubresource = subresourceLayers;

    device->TransferAsync([&](CommandBuffer *copyCmdBuf) -> void {
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount   = 1;
        subresourceRange.layerCount   = 1;

        ImageBarrier barriers[2]{};
        barriers[0].sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[0].image            = *src;
        barriers[0].subresourceRange = subresourceRange;
        barriers[0].srcAccessMask    = 0;
        barriers[0].dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT;
        barriers[0].oldLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barriers[0].newLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        barriers[1].sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[1].image            = *dst;
        barriers[1].subresourceRange = subresourceRange;
        barriers[1].srcAccessMask    = 0;
        barriers[1].dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
        barriers[1].oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[1].newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        copyCmdBuf->PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            SL_ARRAY_LENGTH(barriers), barriers
            );

        copyCmdBuf->CopyImage(
            *src,
            barriers[0].newLayout,
            *dst,
            barriers[1].newLayout,
            1,
            &region
            );

            barriers[0].Swap();
            barriers[1].Swap();

        barriers[1].To(VK_IMAGE_LAYOUT_GENERAL);
        barriers[1].To(VK_ACCESS_HOST_READ_BIT);

        copyCmdBuf->PipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT,
            0,
            0, nullptr,
            0, nullptr,
            SL_ARRAY_LENGTH(barriers), barriers
        );
        });

    dst->Map((void **)&ptr);
    {
        auto &extent = dst->Extent();
        auto align = SLALIGN(extent.width, 8) - extent.width;
        size_t stride = format.Size();

        pixel = *(uint64_t *)(ptr + (y * (align + extent.width) * stride) + x * stride);
    }
    dst->Unmap();

    return pixel;
}

void RenderTarget::Map(uint32_t index, uint8_t **ppData)
{
    THROWIF(index >= attachments.colors.size(), SError::OutOfBound);
    stagingImages[index]->Map((void **)ppData);
}

void RenderTarget::Unmap(uint32_t index)
{
    THROWIF(index >= attachments.colors.size(), SError::OutOfBound);
    stagingImages[index]->Unmap();
}

}
}
