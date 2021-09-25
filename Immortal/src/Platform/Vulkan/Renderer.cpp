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

    queue = context->Get<Queue*>();

    auto &commandPool = device->Get<CommandPool>();
    commandBuffers.resize(frameSize);
    for (auto &buf : commandBuffers)
    {
        buf = commandPool.RequestBuffer(Level::Primary);
    }
    fences.resize(frameSize);
    for (auto &fence : fences)
    {
        fence = device->RequestFence();
    }

    context->Set(commandBuffers);
}

void Renderer::RenderFrame()
{
    if (swapchain)
    {
        LOG::INFO("Transmit Semaphore => {0} to GPU", (void *)semaphores[currentBuffer].acquiredImageReady);
        auto error = swapchain->AcquireNextImage(&currentBuffer, semaphores[currentBuffer].acquiredImageReady, VK_NULL_HANDLE);
        if ((error == VK_ERROR_OUT_OF_DATE_KHR) || (error == VK_SUBOPTIMAL_KHR))
		{
			// resize(width, height);
		}
        currentBuffer = (currentBuffer + 1) % commandBuffers.size();
    }
}

void Renderer::SubmitFrame()
{
    VkResult error{ VK_SUCCESS };

    VkPresentInfoKHR presentInfo{};
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphores[currentBuffer].renderComplete;
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
    LOG::INFO("Waiting for Semaphore => {0} to GPU", (void *)semaphores[currentBuffer].acquiredImageReady);
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores[currentBuffer].acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores[currentBuffer].renderComplete;
    submitInfo.pWaitDstStageMask    = &submitPipelineStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffers[currentBuffer]->Handle();

    queue->Submit(submitInfo, VK_NULL_HANDLE);

    SubmitFrame();
}

}
}
