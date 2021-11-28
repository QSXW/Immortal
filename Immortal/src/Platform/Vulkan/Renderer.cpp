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
    swapchain{ context->Get<Swapchain *>() },
    semaphorePool{ device }
{
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
    auto frameSize = context->FrameSize();
    for (int i = 0; i < frameSize; i++)
    {
        semaphores[i].acquiredImageReady = semaphorePool.Request();
        semaphores[i].renderComplete     = semaphorePool.Request();
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

    sync = (sync + 1) % context->FrameSize();
}

void Renderer::Begin(std::shared_ptr<RenderTarget::Super> &renderTarget)
{
    auto nativeRenderTarget = std::dynamic_pointer_cast<RenderTarget>(renderTarget);
    static VkClearValue clearValues[2];
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].depthStencil = { 0.0f, 100 };

    context->Begin([&](CommandBuffer *cmdbuf) {
        auto &desc = nativeRenderTarget->Desc();

        barriers.image.image = nativeRenderTarget->GetColorImage();

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext                    = nullptr;
        beginInfo.framebuffer              = *nativeRenderTarget->GetAddress<Framebuffer>();
        beginInfo.clearValueCount          = 2;
        beginInfo.pClearValues             = clearValues;
        beginInfo.renderPass               = *nativeRenderTarget->GetAddress<RenderPass>();
        beginInfo.renderArea.extent.width  = desc.Width;
        beginInfo.renderArea.extent.height = desc.Height;
        beginInfo.renderArea.offset        = { 0, 0 };

        cmdbuf->BeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        cmdbuf->SetViewport(ncast<float>(desc.Width), ncast<float>(desc.Height));
        cmdbuf->SetScissor(desc.Width, desc.Height);
    });
}

void Renderer::End()
{
    context->End([&](CommandBuffer *cmdbuf) {
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = 1;

        barriers.image.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers.image.pNext               = nullptr;
        barriers.image.subresourceRange    = subresourceRange;
        barriers.image.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barriers.image.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        barriers.image.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers.image.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barriers.image.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barriers.image.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        

        cmdbuf->EndRenderPass();

        cmdbuf->PipelineBarrier(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barriers.image
        );
    });
}

}
}
