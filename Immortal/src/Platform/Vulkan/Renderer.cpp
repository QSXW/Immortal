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
    auto frameSize = context->FrameSize();
    semaphores.resize(frameSize);
    for (int i = 0; i < frameSize; i++)
    {
        semaphores[i].acquiredImageReady = semaphorePool->Request();
        semaphores[i].renderComplete     = semaphorePool->Request();
    }

    fence = device->RequestFence();

    queue = context->Get<Queue*>();

    auto &commandPool = device->Get<CommandPool>();
    commandBuffer = commandPool.RequestBuffer(Level::Primary, frameSize);

    context->Set<CommandBuffer*>(commandBuffer);
}

void Renderer::RenderFrame()
{
    if (swapchain)
    {
        auto error = swapchain->AcquireNextImage(&currentBuffer, semaphores[semaphoresIndex].acquiredImageReady, VK_NULL_HANDLE);
        if ((error == VK_ERROR_OUT_OF_DATE_KHR) || (error == VK_SUBOPTIMAL_KHR))
		{
			// resize(width, height);
		}
        currentBuffer = (currentBuffer + 1) % commandBuffer->Size();
        semaphoresIndex = (semaphoresIndex + 1) % commandBuffer->Size();
    }
}

void Renderer::SubmitFrame()
{
    VkResult error{ VK_SUCCESS };

    VkPresentInfoKHR presentInfo{};
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphores[semaphoresIndex].renderComplete;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = &swapchain->Handle();
	presentInfo.pImageIndices      = &currentBuffer;
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

    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores[semaphoresIndex].acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores[semaphoresIndex].renderComplete;
    submitInfo.pWaitDstStageMask    = &submitPipelineStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffers[currentBuffer];

    queue->Submit(submitInfo, VK_NULL_HANDLE);

    SubmitFrame();
}

}
}
