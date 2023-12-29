#include "RenderTarget.h"

#include "Image.h"
#include "Barrier.h"
#include "DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

RenderTarget::RenderTarget(Device *device) :
    device{ device },
    renderPass{}
{

}

RenderTarget::RenderTarget() :
    Super{},
    device{},
    renderPass{}
{

}

RenderTarget::~RenderTarget()
{

}

void RenderTarget::Resize(uint32_t width, uint32_t height)
{

}

SuperTexture *RenderTarget::GetColorAttachment(uint32_t index)
{
	return &attachments.colors[index];
}

SuperTexture *RenderTarget::GetDepthAttachment()
{
	return &attachments.depth;
}

void RenderTarget::Release()
{
	framebuffer.Release();
	renderPass.Release();
}

void RenderTarget::SetColorAttachment(uint32_t index, Texture &&texture)
{
    if (index >= attachments.colors.size())
    {
		attachments.colors.resize(index + 1);
    }

    attachments.colors[index] = std::move(texture);
}

void RenderTarget::Construct(std::vector<Texture> &colorAttachments, Texture &&depthAttachment, bool isPresented)
{
	attachments.colors.clear();
	for (size_t i = 0; i < colorAttachments.size(); i++)
    {
		attachments.colors.emplace_back(std::move(colorAttachments[i]));
    }
    
    attachments.depth = std::move(depthAttachment);

    // renderPass = RenderPass{ device, attachments.colors, attachments.depth, isPresented };

    // Invalidate();
}

void RenderTarget::Invalidate()
{
    LightArray<VkImageView> views;

    for (auto &color : attachments.colors)
    {
        views.emplace_back(color.GetImageView());
    }

    if (attachments.depth)
    {
		views.emplace_back(attachments.depth.GetImageView());
    }

    auto &extent = attachments.colors[0].GetExtent();

    framebuffer = std::move(Framebuffer{ device, renderPass, views, VkExtent2D{ extent.width, extent.height } });
}

void RenderTarget::Create()
{
    //VkExtent3D extent = { desc.Width, desc.Height, 1 };
    //VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
    //VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    //attachments.colors.clear();
    //stagingImages.clear();
    //size_t index = 0;
    //for (auto &attachment : desc.Attachments)
    //{
    //    if (attachment.IsDepth())
    //    {
    //        VkClearDepthStencilValue *depthClearValue = reinterpret_cast<VkClearDepthStencilValue *>(&clearValues[index]);
    //        depthClearValue->depth   = 1.0f;
    //        depthClearValue->stencil = 0.0f;
    //        depthFormat = attachment.format;
    //        attachments.depth.image = std::move(Image{
    //            device,
    //            extent,
    //            depthFormat,
    //            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //            VMA_MEMORY_USAGE_GPU_ONLY
    //            });
    //        attachments.depth.view = std::move(ImageView{ &attachments.depth.image, VK_IMAGE_VIEW_TYPE_2D });
    //    }
    //    else
    //    {
    //        VkImageUsageFlags flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    //        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    //        attachments.colors.resize(attachments.colors.size() + 1);
    //        auto &color = attachments.colors.back();
    //        colorFormat = attachment.format;
    //        color.image = std::move(Image{
    //            device,
    //            extent,
    //            colorFormat,
    //            flags,
    //            VMA_MEMORY_USAGE_GPU_ONLY
    //            });
    //        color.view = std::move(ImageView{ &color.image, VK_IMAGE_VIEW_TYPE_2D });

    //        auto image = new Image{
    //            device,
    //            extent,
    //            colorFormat,
    //            VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    //            VMA_MEMORY_USAGE_GPU_TO_CPU,
    //            VK_SAMPLE_COUNT_1_BIT,
    //            1, 1,
    //            VK_IMAGE_TILING_LINEAR
    //            };
    //        uint8_t *dataMapped = nullptr;
    //        image->Map((void **)&dataMapped);
    //        memset(dataMapped, -1, attachment.format.ComponentCount() *extent.width * extent.height * extent.depth);
    //        image->Unmap();
    //        stagingImages.emplace_back(image);
    //    }
    //    index++;
    //}

    //std::vector<VkAttachmentDescription> attchmentDescriptions{};
    //std::vector<VkAttachmentReference> colorRefs{};

    //for (auto &color : attachments.colors)
    //{
    //    VkAttachmentDescription desc{};
    //    desc.format         = color.image.GetFormat();
    //    desc.samples        = VK_SAMPLE_COUNT_1_BIT;
    //    desc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //    desc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    //    desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //    desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //    desc.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    //    attchmentDescriptions.emplace_back(std::move(desc));

    //    VkAttachmentReference colorRef{};
    //    colorRef.attachment = attchmentDescriptions.size() - 1;
    //    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //    colorRefs.emplace_back(std::move(colorRef));
    //}

    //VkAttachmentDescription depthAttachmentDesc{};
    //depthAttachmentDesc.format         = depthFormat;
    //depthAttachmentDesc.samples        = VK_SAMPLE_COUNT_1_BIT;
    //depthAttachmentDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //depthAttachmentDesc.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //depthAttachmentDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //depthAttachmentDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //depthAttachmentDesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //attchmentDescriptions.emplace_back(std::move(depthAttachmentDesc));

    //VkAttachmentReference depthRef{};
    //depthRef.attachment = attchmentDescriptions.size() - 1;
    //depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //VkSubpassDescription subpassDescription{};
    //subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //subpassDescription.colorAttachmentCount    = U32(colorRefs.size());
    //subpassDescription.pColorAttachments       = colorRefs.data();
    //subpassDescription.pDepthStencilAttachment = &depthRef;

    //std::array<VkSubpassDependency, 2> dependencies{};

    //dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass      = 0;
    //dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    //dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    //dependencies[1].srcSubpass      = 0;
    //dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    //dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    //VkRenderPassCreateInfo renderPassInfo = {};
    //renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //renderPassInfo.attachmentCount = U32(attchmentDescriptions.size());
    //renderPassInfo.pAttachments    = attchmentDescriptions.data();
    //renderPassInfo.subpassCount    = 1;
    //renderPassInfo.pSubpasses      = &subpassDescription;
    //renderPassInfo.dependencyCount = U32(dependencies.size());
    //renderPassInfo.pDependencies   = dependencies.data();

    //renderPass = new RenderPass(device, &renderPassInfo);

    Invalidate();
}

}
}
