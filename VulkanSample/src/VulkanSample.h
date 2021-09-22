#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/GuiLayer.h"
#include "Render/Render.h"

using namespace Immortal;

class VulkanLayer : public Layer
{
public:
    VulkanLayer();
    virtual ~VulkanLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnGuiRender() override;

public:
    struct
    {
        bool showDemoWindow{ false };
        bool showAnotherWindow{ false };
        Vector4 clearColor{ 0.45f, 0.55f, 0.60f, 1.00f };
    } Settings;
};

class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ U8("Vulkan Test"), 1920, 1080 })
    {

   
        PushLayer(new VulkanLayer());
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

    

    void CreatePipelineCache()
    {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        Vulkan::Check(vkCreatePipelineCache(context->GetDevice()->Get<VkDevice>(), &createInfo, nullptr, &pipelineCache));
    }

private:
    Vulkan::RenderContext *context{ nullptr };

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
    
    Vulkan::CommandBuffer *drawCommadBuffer;
    
    // std::unique_ptr<Vulkan::FencePool> fences;
    
    struct
    {
        std::unique_ptr<Vulkan::Image> image;
        std::unique_ptr<Vulkan::ImageView> view;
    } depthStencil;

    VkFormat depthFormat;

    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };
};

Immortal::Application* Immortal::CreateApplication()
{
    Render::Set(Render::Type::Vulkan);
    return new VulkanSample();
}
