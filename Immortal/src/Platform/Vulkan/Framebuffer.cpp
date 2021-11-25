#include "impch.h"
#include "Framebuffer.h"

#include "Device.h"
#include "RenderPass.h"
#include "ImageView.h"
#include "Texture.h"
#include <backends/imgui_impl_vulkan.h>

namespace Immortal
{
namespace Vulkan
{

Framebuffer::Framebuffer()
{

}

Framebuffer::Framebuffer(Device *device, const Framebuffer::Description &description) :
    Super{ description },
    device{ device },
    sampler{ device, description.Attachments[0] }
{
    VkExtent3D extent = { desc.Width, desc.Height, 1 };
    VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    std::vector<VkImageView> imageViews;
    for (auto &attachment : description.Attachments)
    {
        if (attachment.IsDepth())
        {
            depthFormat = attachment.BaseFromat<VkFormat>();
            attachments.depth.image.reset(new Image{
                device,
                extent,
                depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY
            });
            attachments.depth.view.reset(new ImageView{ attachments.depth.image.get(), VK_IMAGE_VIEW_TYPE_2D });
            imageViews.emplace_back(*attachments.depth.view);
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
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ,
                VMA_MEMORY_USAGE_GPU_ONLY
            });
            color.view.reset(new ImageView{ color.image.get(), VK_IMAGE_VIEW_TYPE_2D });
            imageViews.emplace_back(*color.view);
        }
    }
    renderPass.reset(new RenderPass{ device, colorFormat, depthFormat, false });

    INIT(imageViews);
    Prepare();
}

Framebuffer::Framebuffer(Device *device, std::shared_ptr<RenderPass> &renderPass, std::vector<ImageView> &views, VkExtent2D &extent) :
    device{ device },
    renderPass{ renderPass }
{
    desc.Width = extent.width;
    desc.Height = extent.height;
    
    std::vector<VkImageView> imageViews;

    for (auto &view : views)
    {
        imageViews.emplace_back(view.Handle());
    }
    INIT(imageViews);
}

Framebuffer::~Framebuffer()
{

}

void Framebuffer::Prepare()
{
    std::array<VkDescriptorSetLayoutBinding, 1> binding{};
    binding[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount    = 1;
    binding[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding[0].binding            = 0;
    binding[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo{};
    descriptorLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutCreateInfo.bindingCount = binding.size();
    descriptorLayoutCreateInfo.pBindings    = binding.data();

    Check(device->Create(&descriptorLayoutCreateInfo, nullptr, &descriptor.setLayout));
    Check(device->AllocateDescriptorSet(&descriptor.setLayout, &descriptor.set));
}

void Framebuffer::Map(uint32_t slot)
{
    VkDescriptorImageInfo image_info{};
    image_info.imageView   = *attachments.colors[0].view;
    image_info.sampler     = sampler;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeDesc{};
    writeDesc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc.pNext           = nullptr;
    writeDesc.dstBinding      = slot;
    writeDesc.dstSet          = descriptor.set;
    writeDesc.descriptorCount = 1;
    writeDesc.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDesc.pImageInfo      = &image_info;

    device->UpdateDescriptorSets(1, &writeDesc, 0, nullptr);
}

void Framebuffer::Unmap()
{

}

void Framebuffer::Resize(UINT32 width, UINT32 height)
{
}

void *Framebuffer::ReadPixel(UINT32 attachmentIndex, int x, int y, Format format, int width, int height)
{
    return nullptr;
}

void Framebuffer::ClearAttachment(UINT32 attachmentIndex, int value)
{
}

void Framebuffer::INIT(std::vector<VkImageView> views)
{
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.attachmentCount = U32(views.size());
    createInfo.pAttachments    = views.data();
    createInfo.renderPass      = *renderPass;
    createInfo.width           = desc.Width;
    createInfo.height          = desc.Height;
    createInfo.layers          = 1;

    Check(vkCreateFramebuffer(device->Handle(), &createInfo, nullptr, &handle));
}

}
}
