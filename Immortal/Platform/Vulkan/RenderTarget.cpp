#include "impch.h"
#include "RenderTarget.h"

#include "Image.h"
#include "RenderContext.h"
#include "DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

std::unique_ptr<RenderTarget> RenderTarget::Create(std::unique_ptr<Image> &&colorImage)
{
    std::vector<std::unique_ptr<Image>> images;

    auto device = colorImage->Get<Device *>();

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

    return std::make_unique<RenderTarget>(std::move(images));
};

RenderTarget::RenderTarget(Device *device, const RenderTarget::Description &description) :
    Super{ description },
    device{ device },
    sampler{ device, description.Attachments[0] }
{
    Create();
}

RenderTarget::RenderTarget(std::vector<std::unique_ptr<Image>> images) :
    device{ images.back()->Get<Device *>() }
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

    SetupExtent(extent);

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
    LOG::WARN(&other == this && "You are trying to self-assigments, which is not acceptable.");
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
    size_t index = 0;
    for (auto &attachment : desc.Attachments)
    {
        if (attachment.IsDepth())
        {
            VkClearDepthStencilValue *depthClearValue = reinterpret_cast<VkClearDepthStencilValue *>(&clearValues[index]);
            depthClearValue->depth   = 1.0f;
            depthClearValue->stencil = 0.0f;
            depthFormat = attachment.BaseFromat<VkFormat>();
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
            attachments.colors.resize(attachments.colors.size() + 1);
            auto &color = attachments.colors.back();
            colorFormat = attachment.BaseFromat<VkFormat>();
            color.image.reset(new Image{
                device,
                extent,
                colorFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY
                });
            color.view.reset(new ImageView{ color.image.get(), VK_IMAGE_VIEW_TYPE_2D });
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

    renderPass.reset(new RenderPass(device, &renderPassInfo));

    SetupFramebuffer();
    SetupDescriptor();
}

void RenderTarget::SetupDescriptor()
{
    if (!descriptor)
    {
       descriptor.reset(new ImageDescriptor{});
    }
    descriptor->Update(sampler, *attachments.colors[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (!descriptorSet)
    {
        descriptorSet.reset(new DescriptorSet{ device, RenderContext::DescriptorSetLayout });
    }
    descriptorSet->Update(*descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

RenderTarget::operator uint64_t() const
{
    return *descriptorSet;
}

void RenderTarget::Map(uint32_t slot)
{

}

void RenderTarget::Resize(uint32_t x, uint32_t y)
{
    desc.Width  = x;
    desc.Height = y;
    
    device->Wait();
    Create();
}

}
}
