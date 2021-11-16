#include "impch.h"
#include "Renderer.h"
#include <array>
#include "..\D3D12\Renderer.h"

namespace Immortal
{
namespace Vulkan
{

Renderer::Renderer(RenderContext::Super *c) : 
    context{ dcast<RenderContext *>(c) },
    device{ context->Get<Device *>() },
    swapchain{ context->Get<Swapchain *>() }
{
    depthFormat   = SuitableDepthFormat(device->Get<VkPhysicalDevice>());
    semaphorePool = std::make_unique<SemaphorePool>(device);
    INIT();
}

Renderer::~Renderer()
{
    for (auto &fence : fences)
    {
        device->Discard(&fence);
    }
}

void Renderer::INIT()
{
    frameSize = context->FrameSize();
    for (int i = 0; i < frameSize; i++)
    {
        semaphores[i].acquiredImageReady = semaphorePool->Request();
        semaphores[i].renderComplete     = semaphorePool->Request();
    }

    queue = context->Get<Queue*>();
}

void Renderer::PrepareFrame()
{
    if (!swapchain)
    {
        return;
    }

    auto error = swapchain->AcquireNextImage(&currentBuffer, semaphores[sync].acquiredImageReady, VK_NULL_HANDLE);
    context->MoveToFrame(currentBuffer);
    if (fences[sync] != VK_NULL_HANDLE)
    {
        device->WaitAndReset(&fences[sync]);
    }
    else
    {
        fences[sync] = device->RequestFence();
    }

    if ((error == VK_ERROR_OUT_OF_DATE_KHR) || (error == VK_SUBOPTIMAL_KHR))
    {
        Resize();
    }
    else
    {
        Check(error);
    }
}

void Renderer::Resize()
{
    swapchain = context->UpdateSurface();
    device->Wait();
}

void Renderer::SubmitFrame()
{
    VkResult error{ VK_SUCCESS };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphores[sync].renderComplete;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain->Handle();
    presentInfo.pImageIndices      = &currentBuffer;
    presentInfo.pResults           = nullptr;

    error = queue->Present(presentInfo);
    if (error == VK_ERROR_OUT_OF_DATE_KHR || error == VK_SUBOPTIMAL_KHR)
    {
        Resize();
        return;
    }
    Check(error);
}

void Renderer::SwapBuffers()
{
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &semaphores[sync].acquiredImageReady;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores[sync].renderComplete;
    submitInfo.pWaitDstStageMask    = &submitPipelineStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &context->GetCommandBuffer()->Handle();

    queue->Submit(submitInfo, fences[sync]);
    SubmitFrame();

    sync = (sync + 1) % frameSize;
}

}
}
