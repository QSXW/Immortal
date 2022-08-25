#include "Renderer.h"
#include <array>

#include "Texture.h"
#include "Submitter.h"

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

    timelineSemaphore = semaphorePool.Request(0);
}

void Renderer::PrepareFrame()
{
    LOG::INFO("PrepareFrame");
    VkResult ret = VK_SUCCESS;
    if (!swapchain)
    {
        VkSurfaceCapabilitiesKHR properties;
        Check(device->GetSurfaceCapabilities(&properties));
        if (properties.currentExtent.width > 0 && properties.currentExtent.height > 0)
        {
            Resize();
        }
    }

    if (swapchain)
    {
        ret = swapchain->AcquireNextImage(&currentBuffer, semaphores[sync].acquiredImageReady, VK_NULL_HANDLE);
        context->MoveToFrame(currentBuffer);
    }

    syncValues[sync] = lastSync;
    if (fences[sync] != VK_NULL_HANDLE)
    {
        LOG::INFO("Wait Fence={}", (void*)fences[sync]);
        device->WaitAndReset(&fences[sync]);
    }
    else
    {
        fences[sync] = device->RequestFence();
    }

    if ((ret == VK_ERROR_OUT_OF_DATE_KHR) || (ret == VK_SUBOPTIMAL_KHR))
    {
        Resize();
    }
    else
    {
        Check(ret);
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
    PresentSubmitter submitter{};
    submitter.Wait(semaphores[sync].renderComplete);
    submitter.Present(*swapchain, currentBuffer);

    VkResult ret = queue->Present(submitter);
    if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR)
    {
        Resize();
        return;
    }
    Check(ret);
}

void Renderer::SwapBuffers()
{
    LOG::INFO("SwapBuffers");
    LOG::INFO("queue={}, sync={}, semaphore={}, fence={}", (void*)queue->Handle(), sync, (void*)semaphores[sync].renderComplete, (void*)fences[sync]);
    
    uint64_t signalValues[3] = { 0 };

    {
        device->TransferAsync([&](CommandBuffer *cmdbuf) {
            cmdbuf->End();

            signalValues[0] = ++syncValues[sync];

            TimelineSubmitter timelineSubmitter{};
            timelineSubmitter.Signal(signalValues[0]);

            Submitter submitter{};
            submitter.SignalSemaphore(timelineSemaphore);
            submitter.Execute(cmdbuf);
            submitter.Trampoline(timelineSubmitter);

            auto &transferQueue = device->FindQueueByType(Queue::Type::Transfer, device->QueueFailyIndex(Queue::Type::Transfer));
            transferQueue.Submit(submitter, nullptr);
            transferQueue.Wait();
            });

        device->ComputeAsync([&](CommandBuffer *cmdbuf) {
            cmdbuf->End();
            signalValues[1] = ++syncValues[sync];

            TimelineSubmitter timelineSubmitter{};
            timelineSubmitter.Wait(signalValues[0]);
            timelineSubmitter.Signal(signalValues[1]);

            Submitter submitter{};
            submitter.WaitSemaphore(timelineSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            submitter.SignalSemaphore(timelineSemaphore);
            submitter.Execute(cmdbuf);
            submitter.Trampoline(timelineSubmitter);

            auto &computeQueue = device->FindQueueByType(Queue::Type::Compute, device->QueueFailyIndex(Queue::Type::Compute));
            computeQueue.Submit(submitter, nullptr);
            });
    }

    context->Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->End();

        uint64_t graphicsSignalValue = ++syncValues[sync];

        TimelineSubmitter timelineSubmitter{};
        timelineSubmitter.Wait(signalValues[1]);
        timelineSubmitter.Signal(graphicsSignalValue);

        Submitter submitter{};
        submitter.WaitSemaphore(timelineSemaphore, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
        submitter.SignalSemaphore(timelineSemaphore);
        submitter.Execute(*cmdbuf);

        if (swapchain)
        {
            timelineSubmitter.Wait(0);
            timelineSubmitter.Signal(0);

            submitter.WaitSemaphore(semaphores[sync].acquiredImageReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            submitter.SignalSemaphore(semaphores[sync].renderComplete);
        }

        submitter.Trampoline(timelineSubmitter);
        queue->Submit(submitter, fences[sync]);
        });

    if (swapchain)
    {
        SubmitFrame();
    }

    lastSync = syncValues[sync];
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
        beginInfo.framebuffer              = renderTarget->Get<Framebuffer>();
        beginInfo.clearValueCount          = renderTarget->ColorAttachmentCount() + 1;
        beginInfo.pClearValues             = rcast<VkClearValue*>(&renderTarget->clearValues);
        beginInfo.renderPass               = renderTarget->Get<RenderPass>();
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

GraphicsPipeline::Super *Renderer::CreateGraphicsPipeline(Ref<Shader::Super> shader)
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
