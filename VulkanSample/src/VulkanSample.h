#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/GuiLayer.h"

using namespace Immortal;

class VulkanLayer : public Layer
{
public:
    VulkanLayer();
    virtual ~VulkanLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
};

class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Vulkan Test"), 1920, 1080 })
    {
        context = dcast<Vulkan::RenderContext *>(Context());
        auto device = context->GetDevice();
        VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        VkDevice deviceHandle = rcast<VkDevice>(device->Handle());

        Vulkan::Check(vkCreateSemaphore(deviceHandle, &createInfo, nullptr, &semaphores.acquiredImageReady));
        Vulkan::Check(vkCreateSemaphore(deviceHandle, &createInfo, nullptr, &semaphores.renderComplete));

        queue = rcast<VkQueue>(device->SuitableGraphicsQueue().Handle());
        CreateSwapchainBuffers();
        
        auto frameSize = ncast<UINT32>(context->Get<Vulkan::RenderContext::Frames>().size());
    
        commandPool.reset(new Vulkan::CommandPool{ device, device->FindQueueByFlags(Vulkan::Queue::Graphics | Vulkan::Queue::Compute, 0).Get<Vulkan::Queue::FamilyIndex>() });
        drawCommadBuffer.reset(new Vulkan::CommandBuffer{ 
            commandPool.get(),
            Vulkan::CommandBuffer::Level::Primary,
            frameSize
        });

        fences.reset(new Vulkan::FencePool{ device });
        for (UINT32 i = 0; i < frameSize; i++)
        {
            fences->Request();
        }

        extent = Vulkan::Extent3D{ context->Get<Vulkan::Extent2D>().width, context->Get<Vulkan::Extent2D>().height, 1 };

        depthFormat = Vulkan::SuitableDepthFormat(device->Get<VkPhysicalDevice>());

        depthStencil.image.reset(new Vulkan::Image {
            device,
            extent,
            depthFormat,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY
        });

        depthStencil.view.reset(new Vulkan::ImageView {
            depthStencil.image.get(),
            VK_IMAGE_VIEW_TYPE_2D
        });

        SetupRenderPass();
        CreatePipelineCache();
        SetupFramebuffer();

        PushLayer(new Vulkan::GuiLayer(context, renderPass));
    }

    ~VulkanSample()
    {

    }

    void CreateSwapchainBuffers()
    {
        auto &device = context->Get<Vulkan::Device>();
        if (context->HasSwapchain())
        {
            auto &swapchain = context->Get<Vulkan::Swapchain>();
            auto &images    = swapchain.Get<Vulkan::Swapchain::Images>();

            for (auto &s : swapchainBuffers)
            {
                Vulkan::IfNotNullThen(vkDestroyImageView, device.Get<VkDevice>(), s.view);
            }
            swapchainBuffers.clear();
            swapchainBuffers.resize(images.size());

            for (auto i = 0; i < images.size(); i++)
            {
                VkImageViewCreateInfo colorAttachmentView{};
                colorAttachmentView.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                colorAttachmentView.pNext      = nullptr;
                colorAttachmentView.format     = swapchain.Get<VkFormat>();
                colorAttachmentView.components = {
                    VK_COMPONENT_SWIZZLE_R,
                    VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B,
                    VK_COMPONENT_SWIZZLE_A
                };
                colorAttachmentView.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                colorAttachmentView.subresourceRange.baseMipLevel   = 0;
                colorAttachmentView.subresourceRange.levelCount     = 1;
                colorAttachmentView.subresourceRange.baseArrayLayer = 0;
                colorAttachmentView.subresourceRange.layerCount     = 1;
                colorAttachmentView.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
                colorAttachmentView.flags                           = 0;

                swapchainBuffers[i].image = images[i];
                colorAttachmentView.image = images[i];
                Vulkan::Check(vkCreateImageView(device.Get<VkDevice>(), &colorAttachmentView, nullptr, &swapchainBuffers[i].view));
            }
        }
    }

    void SetupRenderPass()
    {
        std::array<VkAttachmentDescription, 2> attachments{};

        auto &colorAttachment          = attachments[0];
        colorAttachment.format         = context->Get<VkFormat>();
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        auto &depthAttachment          = attachments[1];
        depthAttachment.format         = depthFormat;
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthReference{};
        depthReference.attachment = 1;
        depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount    = 1;
        subpassDescription.pColorAttachments       = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = nullptr;
        subpassDescription.pResolveAttachments     = nullptr;

        std::array<VkSubpassDependency, 2> dependencies{};

        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = 0;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = ncast<UINT32>(attachments.size());
        createInfo.pAttachments    = attachments.data();
        createInfo.subpassCount    = 1;
        createInfo.pSubpasses      = &subpassDescription;
        createInfo.dependencyCount = ncast<UINT32>(dependencies.size());
        createInfo.pDependencies   = dependencies.data();

        Vulkan::Check(vkCreateRenderPass(context->Get<VkDevice>(), &createInfo, nullptr, &renderPass));
    }

    void CreatePipelineCache()
    {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        Vulkan::Check(vkCreatePipelineCache(context->GetDevice()->Get<VkDevice>(), &createInfo, nullptr, &pipelineCache));
    }

    void SetupFramebuffer()
    {
        std::array<VkImageView, 2> attachments;
        attachments[1] = depthStencil.view->Get<VkImageView>();

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext           = nullptr;
        createInfo.renderPass      = renderPass;
        createInfo.attachmentCount = 2;
        createInfo.pAttachments    = attachments.data();
        createInfo.width           = Application::Width();
        createInfo.height          = Application::Height();
        createInfo.layers          = 1;

        framebuffers.resize(context->Get<Vulkan::RenderContext::Frames>().size());
        for (int i = 0; i < framebuffers.size(); i++)
        {
            attachments[0] = swapchainBuffers[i].view;
            Vulkan::Check(vkCreateFramebuffer(context->GetDevice()->Get<VkDevice>(), &createInfo, nullptr, &framebuffers[i]));
        }
    }

private:
    Vulkan::RenderContext *context{ nullptr };

    struct
    {
        VkSemaphore acquiredImageReady;
        VkSemaphore renderComplete;
    } semaphores;

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkQueue queue;

    struct SwapchainBuffer
    {
        VkImage     image;
        VkImageView view;
    };

    VkExtent3D extent{};

    std::vector<SwapchainBuffer> swapchainBuffers{};

    std::unique_ptr<Vulkan::CommandPool> commandPool;
    
    std::unique_ptr<Vulkan::CommandBuffer> drawCommadBuffer;
    
    std::unique_ptr<Vulkan::FencePool> fences;
    
    struct
    {
        std::unique_ptr<Vulkan::Image> image;
        std::unique_ptr<Vulkan::ImageView> view;
    } depthStencil;

    VkFormat depthFormat;

    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

    std::vector<VkFramebuffer> framebuffers;
};

Immortal::Application* Immortal::CreateApplication()
{
    RendererAPI::SetAPI(RendererAPI::Type::VulKan);
    return new VulkanSample();
}
