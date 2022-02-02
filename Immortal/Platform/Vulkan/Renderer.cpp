#include "impch.h"
#include "Renderer.h"
#include <array>

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
    Setup();
}

Renderer::~Renderer()
{
    for (auto &fence : fences)
    {
        device->Discard(&fence);
    }
}

void Renderer::Setup()
{
    auto frameSize = context->FrameSize();
    for (int i = 0; i < frameSize; i++)
    {
        semaphores[i].acquiredImageReady = semaphorePool.Request();
        semaphores[i].renderComplete     = semaphorePool.Request();
        semaphores[i].compute            = semaphorePool.Request();
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
    context->GetCommandBuffer()->Begin();
    device->BeginComputeThread();
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
    VkPipelineStageFlags computeWaitDstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    VkSubmitInfo computeSubmitInfo{};
    computeSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.pSignalSemaphores    = &semaphores[sync].compute;
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pWaitDstStageMask    = &computeWaitDstStageMask;

    auto &computeQueue = device->FindQueueByType(Queue::Type::Compute, device->QueueFailyIndex(Queue::Type::Compute));
    device->Compute([&](CommandBuffer *cmdbuf) {
        cmdbuf->End();
        computeSubmitInfo.commandBufferCount = 1;
        computeSubmitInfo.pCommandBuffers = &cmdbuf->Handle();
        });

    computeQueue.Submit(computeSubmitInfo, nullptr);

    context->GetCommandBuffer()->End();

    VkSemaphore waitSemaphores[] = {
        semaphores[sync].compute,
        semaphores[sync].acquiredImageReady
    };

    static VkPipelineStageFlags graphicsWaitDstStageMask[] = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount   = SL_ARRAY_LENGTH(waitSemaphores);
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores[sync].renderComplete;
    submitInfo.pWaitDstStageMask    = graphicsWaitDstStageMask;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &context->GetCommandBuffer()->Handle();

    queue->Submit(submitInfo, fences[sync]);
    SubmitFrame();

    sync = (sync + 1) % context->FrameSize();

    device->ExecuteComputeThread();
    device->DestroyObjects();
}

void Renderer::Begin(std::shared_ptr<RenderTarget::Super> &superRenderTarget)
{
    auto renderTarget = std::dynamic_pointer_cast<RenderTarget>(superRenderTarget);

    context->Begin([&](CommandBuffer *cmdbuf) {
        auto &desc = renderTarget->Desc();

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext                    = nullptr;
        beginInfo.framebuffer              = renderTarget->GetFramebuffer();
        beginInfo.clearValueCount          = renderTarget->ColorAttachmentCount() + 1;
        beginInfo.pClearValues             = rcast<VkClearValue*>(&renderTarget->clearValues);
        beginInfo.renderPass               = renderTarget->GetRenderPass();
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
        cmdbuf->EndRenderPass();
    });
}

void Renderer::PushConstant(GraphicsPipeline::Super *superPipeline, Shader::Stage stage, uint32_t size, const void * data, uint32_t offset)
{
    auto pipeline = dynamic_cast<GraphicsPipeline *>(superPipeline);
    context->Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->PushConstants(
            pipeline->Layout(),
            stage,
            offset,
            size,
            data
            );
        });
}

void Renderer::PushConstant(ComputePipeline::Super *superPipeline, Shader::Stage stage, uint32_t size, const void * data, uint32_t offset)
{
    auto pipeline = dynamic_cast<ComputePipeline *>(superPipeline);
    context->Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->PushConstants(
            pipeline->Layout(),
            stage,
            offset,
            size,
            data
            );
        });
}

void Renderer::Draw(GraphicsPipeline::Super *superPipeline)
{
    context->Submit([&](CommandBuffer *cmdbuf) {
        auto pipeline = dynamic_cast<GraphicsPipeline *>(superPipeline);
        cmdbuf->BindDescriptorSets(
            pipeline->BindPoint(),
            pipeline->Layout(),
            0, 1,
            &pipeline->GetDescriptorSet(),
            0, 0
            );

        cmdbuf->BindPipeline(
            *pipeline,
             pipeline->BindPoint()
            );

        auto &buffer = pipeline->Get<Buffer::Type::Vertex>();
        cmdbuf->BindVertexBuffers(
            0, 1,
            &buffer->Handle(),
            &buffer->Offset()
            );

        buffer = pipeline->Get<Buffer::Type::Index>();
        cmdbuf->BindIndexBuffer(
            *buffer,
             buffer->Offset()
            );

        cmdbuf->DrawIndexed(
            pipeline->ElementCount,
            1, 0, 0, 0
            );
    });
}

void Renderer::Dispatch(ComputePipeline::Super *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
    context->Submit([&](CommandBuffer *cmdbuf) {
        auto pipeline = dynamic_cast<ComputePipeline *>(superPipeline);
        pipeline->Dispatch(cmdbuf, nGroupX, nGroupY, nGroupZ);
        });
}

Descriptor *Renderer::CreateImageDescriptor(uint32_t count)
{
    auto descriptor = new ImageDescriptor[count];
    CleanUpObject(descriptor);

    return descriptor;
}

Descriptor *Renderer::CreateBufferDescriptor(uint32_t count)
{
    auto descriptor = new BufferDescriptor[count];
    CleanUpObject(descriptor);

    return descriptor;
}

}
}
