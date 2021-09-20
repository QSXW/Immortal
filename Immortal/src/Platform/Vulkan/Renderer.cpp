#include "impch.h"
#include "Renderer.h"
#include <array>

namespace Immortal
{
namespace Vulkan
{

Renderer::Renderer(RenderContext::Super *c) : 
    context{ dcast<RenderContext *>(c) },
    device{ &context->Get<Device>() },
    swapchain{ &context->Get<Swapchain>() }
{
    depthFormat   = SuitableDepthFormat(device->Get<VkPhysicalDevice>());
    semaphorePool = std::make_unique<SemaphorePool>(device);
    INIT();
}

void Renderer::INIT()
{
    semaphores.acquiredImageReady = semaphorePool->Request();
    semaphores.renderComplete     = semaphorePool->Request();

    submitInfo.pWaitDstStageMask    = &submitPipelineStages;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores.acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores.renderComplete;

    queue = device->SuitableGraphicsQueuePtr();

    auto &commandPool = device->Get<CommandPool>();
    commandBuffer = commandPool.RequestBuffer(Level::Primary);
    context->Set<CommandBuffer*>(commandBuffer);
}

void Renderer::RenderFrame()
{
    if (swapchain)
    {
        swapchain->AcquireNextImage(frameIndex, semaphores.acquiredImageReady, VK_NULL_HANDLE);
    }
}

void Renderer::SubmitFrame()
{
    VkResult error{ VK_SUCCESS };

    VkPresentInfoKHR presentInfo{};
	presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext            = nullptr;
	presentInfo.swapchainCount   = 1;
	presentInfo.pSwapchains      = &swapchain->Handle();
	presentInfo.pImageIndices    = &frameIndex;

    if (semaphores.renderComplete != VK_NULL_HANDLE)
    {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &semaphores.renderComplete;
    }

    error = queue->Present(presentInfo);
    if (error == VK_ERROR_OUT_OF_DATE_KHR)
    {
        LOG::WARN("Swap chain is no longer compatible with the surface and needs to be recreated");
        return;
    }
    Check(error);
}

void Renderer::SwapBuffers()
{
    RenderFrame();

    auto &commandPool = device->Get<CommandPool>();

    submitInfo.commandBufferCount = commandBuffer->Size();
    submitInfo.pCommandBuffers    = commandBuffer->Data();

    vkQueueSubmit(queue->Handle(), 1, &submitInfo, VK_NULL_HANDLE);

    SubmitFrame();
}

//void SetupFramebuffer()
//{
//    std::array<VkImageView, 2> attachments;
//    attachments[1] = depthStencil.view->Get<VkImageView>();
//
//    VkFramebufferCreateInfo createInfo{};
//    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//    createInfo.pNext           = nullptr;
//    createInfo.renderPass      = renderPass;
//    createInfo.attachmentCount = 2;
//    createInfo.pAttachments    = attachments.data();
//    createInfo.width           = Application::Width();
//    createInfo.height          = Application::Height();
//    createInfo.layers          = 1;
//
//    framebuffers.resize(context->Get<Vulkan::RenderContext::Frames>().size());
//    for (int i = 0; i < framebuffers.size(); i++)
//    {
//        attachments[0] = swapchainBuffers[i].view;
//        Vulkan::Check(vkCreateFramebuffer(context->GetDevice()->Get<VkDevice>(), &createInfo, nullptr, &framebuffers[i]));
//    }
//}

}
}
