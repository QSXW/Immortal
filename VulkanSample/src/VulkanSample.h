#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/vkcommon.h"
#include "Platform/Vulkan/RenderContext.h"

using namespace Immortal;

class VulkanLayer : public Layer
{
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

        queue = rcast<VkQueue>(device->SuitableQueue());
        CreateSwapchainBuffers();

        commandPool.reset(new Vulkan::CommandPool{ device, device->FindQueueByFlags(Vulkan::Queue::Graphics | Vulkan::Queue::Compute, 0).Get<Vulkan::Queue::FamilyIndex>() });
        drawCommadBuffer.reset();

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

    std::vector<SwapchainBuffer> swapchainBuffers{};

    std::unique_ptr<Vulkan::CommandPool> commandPool;
    std::unique_ptr<Vulkan::CommandBuffer> drawCommadBuffer;
};

Immortal::Application* Immortal::CreateApplication()
{
    RendererAPI::SetAPI(RendererAPI::Type::VulKan);
    return new VulkanSample();
}
