#include "RenderPass.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

RenderPass::RenderPass() :
    Handle{},
    device{}
{

}

RenderPass::RenderPass(Device *device, const std::vector<Texture> &colorAttachments, const Texture &depthAttachment, bool isPresented) :
    Handle{},
    device{ device }
{
    LightArray<VkAttachmentDescription> attachments{};
	LightArray<VkAttachmentReference> colorRefs{};

    auto count = colorAttachments.size();
	for (uint32_t i = 0; i < count; i++)
    {
        attachments.emplace_back({
            .flags          = {},
            .format         = colorAttachments[i].GetFormat(),
            .samples        = colorAttachments[i].GetSamples(),
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
		    .finalLayout    = isPresented ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        });

        colorRefs.emplace_back(VkAttachmentReference {
			.attachment = i,
		    .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            });
    }
    
    if (depthAttachment)
    {
		attachments.emplace_back({
            .format         = depthAttachment.GetFormat(),
            .samples        = depthAttachment.GetSamples(),
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        });
    };

    VkAttachmentReference colorReference{};
    colorReference.attachment = count;
    colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference{};
    depthReference.attachment = 1;
    depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount    = uint32_t(colorRefs.size());
    subpassDescription.pColorAttachments       = colorRefs.data();
	subpassDescription.pDepthStencilAttachment = depthAttachment ? &depthReference : nullptr;
    subpassDescription.inputAttachmentCount    = 0;
    subpassDescription.pInputAttachments       = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments    = nullptr;
    subpassDescription.pResolveAttachments     = nullptr;

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

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext           = nullptr;
    createInfo.attachmentCount = U32(attachments.size());
    createInfo.pAttachments    = attachments.data();
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpassDescription;
    createInfo.dependencyCount = U32(dependencies.size());
    createInfo.pDependencies   = dependencies.data();

    Check(device->Create(&createInfo, &handle));
}

RenderPass::RenderPass(Device *device, const VkRenderPassCreateInfo *pCreateInfo) :
    Handle{},
    device{device}
{
	Check(device->Create(pCreateInfo, &handle));
}

RenderPass::~RenderPass()
{
    Release();
}

void RenderPass::Invalidate(VkRenderPass other)
{
    Release();
    handle = other;
}

void RenderPass::Release()
{
    if (device)
    {
        device->DestroyAsync(handle);
		handle = VK_NULL_HANDLE;
		device = nullptr;
    }
}

}
}
