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
    VkPipelineStageFlags pipelineStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    semaphores.acquiredImageReady = semaphorePool->Request();
    semaphores.renderComplete     = semaphorePool->Request();

    submitInfo.pWaitDstStageMask    = &submitPipelineStages;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores.acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores.renderComplete;
    submitInfo.pWaitDstStageMask    = &pipelineStage;

    queue = device->SuitableGraphicsQueuePtr();

    auto &commandPool = device->Get<CommandPool>();
    commandBuffer = commandPool.RequestBuffer(Level::Primary);
    context->Set<CommandBuffer*>(commandBuffer);
}

void Renderer::RenderFrame()
{
    if (swapchain)
    {
        swapchain->AcquireNextImage(&frameIndex, semaphores.acquiredImageReady, VK_NULL_HANDLE);
    }
}

void Renderer::SubmitFrame()
{
    VkResult error{ VK_SUCCESS };

    VkPresentInfoKHR presentInfo{};
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphores.renderComplete;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = &swapchain->Handle();
	presentInfo.pImageIndices      = &frameIndex;
    presentInfo.pResults           = nullptr;

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

    submitInfo.commandBufferCount = commandBuffer->Size();
    submitInfo.pCommandBuffers    = commandBuffer->Data();

    vkQueueSubmit(queue->Handle(), 1, &submitInfo, VK_NULL_HANDLE);

    SubmitFrame();
}

}
}
