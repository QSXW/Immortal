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
        context = Context();
        auto device = context->GetDevice();
        VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        VkDevice deviceHandle = rcast<VkDevice>(device->Handle());

        Vulkan::Check(vkCreateSemaphore(deviceHandle, &createInfo, nullptr, &semaphores.acquiredImageReady));
        Vulkan::Check(vkCreateSemaphore(deviceHandle, &createInfo, nullptr, &semaphores.renderComplete));

        queue = rcast<VkQueue>(device->SuitableQueue());
        CreateSwapchainBuffers();
    }

    ~VulkanSample()
    {

    }

    void CreateSwapchainBuffers()
    {
        auto renderContext = dcast<Vulkan::RenderContext *>(context);

        if (renderContext->HasSwapchain())
        {
            auto &swapchain = renderContext->Get<Vulkan::Swapchain>();
            auto &images    = swapchain.Get<Vulkan::Swapchain::Images>();
        }
    }

private:
    RenderContext *context{ nullptr };

    struct
    {
        VkSemaphore acquiredImageReady;
        VkSemaphore renderComplete;
    } semaphores;

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkQueue queue;
};

Immortal::Application* Immortal::CreateApplication()
{
    RendererAPI::SetAPI(RendererAPI::Type::VulKan);
    return new VulkanSample();
}
