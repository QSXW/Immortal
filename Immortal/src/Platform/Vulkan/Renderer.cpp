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

    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores.acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores.renderComplete;
    submitInfo.pWaitDstStageMask    = &submitPipelineStages;

    queue = context->Get<Queue*>();

    auto &commandPool = device->Get<CommandPool>();
    commandBuffer = commandPool.RequestBuffer(Level::Primary, context->FrameSize());

    context->Set<CommandBuffer*>(commandBuffer);
}

void Renderer::RenderFrame()
{
    if (swapchain)
    {
        swapchain->AcquireNextImage(&frameIndex, semaphores.acquiredImageReady, VK_NULL_HANDLE);
        frameIndex = (frameIndex + 1) % commandBuffer->Size();
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
    VkCommandBuffer *commandBuffers = commandBuffer->Data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffers[frameIndex];

    queue->Submit(submitInfo, VK_NULL_HANDLE);

    SubmitFrame();
}

}
}
