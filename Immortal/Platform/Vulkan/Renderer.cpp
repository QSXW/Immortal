#include "impch.h"
#include "Renderer.h"
#include <array>

#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

Renderer::Renderer(RenderContext::Super *c) :
    context{ dcast<RenderContext *>(c) },
    device{ context->GetAddress<Device>() },
    swapchain{ context->GetAddress<Swapchain>() },
    queue{ context->GetAddress<Queue>() },
    semaphorePool{ device }
{
    Setup();
}

Renderer::~Renderer()
{
    if (!fences[0])
    {
        return;
    }
    device->Wait(fences.data(), fences.size());
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
        semaphores[i].transfer           = semaphorePool.Request();
    }
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

    VkSwapchainKHR swapchainKHR = *swapchain;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphores[sync].renderComplete;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchainKHR;
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
    VkCommandBuffer commandBuffers[1] = { 0 };

    {
        VkPipelineStageFlags waitDstStageFlags[1] = {};

        VkSemaphore signalSemaphores[] = {
            semaphores[sync].compute,
            semaphores[sync].transfer,
        };

        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = commandBuffers;
  
        device->TransferAsync([&](CommandBuffer *cmdbuf) {
            cmdbuf->End();
            waitDstStageFlags[0]         = VK_PIPELINE_STAGE_TRANSFER_BIT;
            submitInfo.pSignalSemaphores = &signalSemaphores[1];
            submitInfo.pWaitDstStageMask = waitDstStageFlags;
            commandBuffers[0] = *cmdbuf;
            });
        
        auto &transferQueue = device->FindQueueByType(Queue::Type::Transfer, device->QueueFailyIndex(Queue::Type::Transfer));
        transferQueue.Submit(submitInfo, nullptr);

        device->ComputeAsync([&](CommandBuffer *cmdbuf) {
            cmdbuf->End();
            waitDstStageFlags[0]          = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            submitInfo.pWaitSemaphores    = &signalSemaphores[1];
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pSignalSemaphores  = &signalSemaphores[0];
            submitInfo.pWaitDstStageMask  = waitDstStageFlags;
            commandBuffers[0] = *cmdbuf;
            });

        auto &computeQueue = device->FindQueueByType(Queue::Type::Compute, device->QueueFailyIndex(Queue::Type::Compute));
        computeQueue.Submit(submitInfo, nullptr);
    }
   
    VkSemaphore waitSemaphores[] = {
        semaphores[sync].compute,
        semaphores[sync].acquiredImageReady
    };

    static VkPipelineStageFlags graphicsWaitDstStageMask[] = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    context->Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->End();
        commandBuffers[0] = *cmdbuf;
        });

    submitInfo.waitSemaphoreCount   = SL_ARRAY_LENGTH(waitSemaphores);
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semaphores[sync].renderComplete;
    submitInfo.pWaitDstStageMask    = graphicsWaitDstStageMask;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = commandBuffers;

    queue->Submit(submitInfo, fences[sync]);
    SubmitFrame();

    SLROTATE(sync, context->FrameSize());

    device->ExecuteComputeThread();
    device->DestroyObjects();
}

void Renderer::Begin(RenderTarget::Super *superRenderTarget)
{
    auto renderTarget = dynamic_cast<RenderTarget *>(superRenderTarget);

    context->Submit([&](CommandBuffer *cmdbuf) {
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
    context->Submit([&](CommandBuffer *cmdbuf) {
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

        VkDescriptorSet descriptorSets[] = { pipeline->GetDescriptorSet() };
        cmdbuf->BindDescriptorSets(
            GraphicsPipeline::BindPoint,
            pipeline->Layout(),
            0, 1,
            descriptorSets,
            0, 0
            );

        cmdbuf->BindPipeline(
            *pipeline,
             GraphicsPipeline::BindPoint
            );

        Ref<Buffer> buffer = pipeline->Get<Buffer::Type::Vertex>();
        VkBuffer vertex = *buffer;
        VkDeviceSize offsets = buffer->Offset();
        cmdbuf->BindVertexBuffers(
            0, 1,
            &vertex,
            &offsets
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

const char *Renderer::GraphicsRenderer()
{
    return context->GraphicsRenderer();
}

Shader::Super *Renderer::CreateShader(const std::string &filepath, Shader::Type type)
{
    return new Shader{ device, filepath, type };
}

GraphicsPipeline::Super *Renderer::CreateGraphicsPipeline(std::shared_ptr<Shader::Super> shader)
{
    return new GraphicsPipeline{ device, shader };
}

ComputePipeline::Super *Renderer::CreateComputePipeline(Shader::Super *shader)
{
    return new ComputePipeline{ device, shader };
}

Texture::Super *Renderer::CreateTexture(const std::string &filepath, const Texture::Description &description)
{
    return new Texture{ device, filepath, description };
}

Texture::Super *Renderer::CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description)
{
    return new Texture{ device, width, height, data, description };
}

TextureCube::Super *Renderer::CreateTextureCube(uint32_t width, uint32_t height, const Texture::Description &description)
{
    return new TextureCube{ device, width, height, description };
}

Buffer::Super *Renderer::CreateBuffer(const size_t size, const void *data, Buffer::Type type)
{
    return new Buffer{ device, size, data, type };
}

Buffer::Super *Renderer::CreateBuffer(const size_t size, Buffer::Type type)
{
    return new Buffer{ device, size, type };
}

Buffer::Super *Renderer::CreateBuffer(const size_t size, uint32_t binding)
{
    return new Buffer{ device, size, binding };
}

RenderTarget::Super *Renderer::CreateRenderTarget(const RenderTarget::Description &description)
{
    return new RenderTarget{ device, description };
}

}
}
