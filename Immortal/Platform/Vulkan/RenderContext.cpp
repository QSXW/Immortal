#include "RenderContext.h"

#include <array>

#include "Common.h"

#include "Framework/Application.h"
#include "RenderContext.h"
#include "Image.h"
#include "Framebuffer.h"
#include "GuiLayer.h"

#if !defined(VK_SYNC_TIMELINE_SEMAPHORE) && !defined(VK_SYNC_FENCE)
#define VK_SYNC_TIMELINE_SEMAPHORE
#endif

namespace Immortal
{
namespace Vulkan
{

VkResult RenderContext::Status = VK_NOT_READY;

std::unordered_map<const char *, bool> RenderContext::InstanceExtensions{

};

std::unordered_map<const char *, bool> RenderContext::DeviceExtensions{
    { VK_KHR_SWAPCHAIN_EXTENSION_NAME,                false },
    { VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,       false },
#ifdef __APPLE__
    { VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,       false },
#endif
    { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,   true },
    { VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,     true },
    { VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,    true },
    { VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, true },
    { VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,      true },
    { VK_KHR_SPIRV_1_4_EXTENSION_NAME,                true },
    { VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,    true },
    { VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,        true },
    { VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,              true },
    { VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,       true },
    { VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME,       true },
    { VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, true },
};

static std::vector<const char *> ValidationLayers = {
#ifdef _DEBUG
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2",
    "VK_LAYER_LIGHTWSI_Light",
#endif
};

RenderContext::RenderContext(const RenderContext::Description &desc) :
    window{ desc.window }
{
    switch (window->GetType())
    {
    case Window::Type::Win32:
        InstanceExtensions.insert({ "VK_KHR_win32_surface", false });
        break;

    case Window::Type::Wayland:
        InstanceExtensions.insert({ "VK_KHR_wayland_surface", false });
        break;

    case Window::Type::XCB:
        InstanceExtensions.insert({ "VK_KHR_xcb_surface", false });
        break;

    case Window::Type::X11:
        InstanceExtensions.insert({ "VK_KHR_xlib_surface", false });
        break;

    case Window::Type::Headless:
        InstanceExtensions.insert({"VK_EXT_headless_surface", false});
        break;

#if defined(__APPLE__)
    case Window::Type::Cocoa:
        InstanceExtensions.insert({ "VK_EXT_metal_surface", false });
        InstanceExtensions.insert({ "VK_KHR_portability_enumeration", false });
        break;
#endif

    default:
        break;
    }

    instance = Instance{ "Immortal Graphics API", InstanceExtensions, ValidationLayers};
    if (!instance)
    {
        LOG::ERR("Vulkan Not Supported!");
        return;
    }

    Check(instance.CreateSurface(window, &surface));

    auto physicalDevice = instance.GetSuitablePhysicalDevice(desc.deviceId);
    physicalDevice->Activate(PhysicalDevice::Feature::SamplerAnisotropy);
    physicalDevice->Activate(PhysicalDevice::Feature::RobustBufferAccess);
    physicalDevice->Activate(PhysicalDevice::Feature::IndependentBlend);

    device = new Device{ physicalDevice, surface, DeviceExtensions };
    queue  = device->SuitableGraphicsQueuePtr();

    if (surface != VK_NULL_HANDLE)
    {
        surfaceExtent = { desc.width, desc.height };
        VkFormat depthFormat = physicalDevice->GetSuitableDepthFormat();
        swapchainPool[0] = std::move(Swapchain{ swapchainPool[0], device, surfaceExtent, desc.presentMode });
        renderPass = new RenderPass{ device, swapchainPool[0].Get<VkFormat>(), depthFormat };
        Prepare();
    }

	commandPool = new TimelineCommandPool{ device, queue->Get<Queue::FamilyIndex>() };

    EnableGlobal();

    Super::UpdateMeta(physicalDevice->Properties.deviceName, nullptr, nullptr);
}

RenderContext::~RenderContext()
{
    if (DescriptorSetLayout != VK_NULL_HANDLE)
    {
        device->DestroyAsync(DescriptorSetLayout);
        DescriptorSetLayout = VK_NULL_HANDLE;
    }

    for (int i = 0; i < FrameSize(); i++)
	{
		device->Release(std::move(semaphores[i].acquiredImageReady));
		device->Release(std::move(semaphores[i].renderComplete));
	}
	device->Release(std::move(timelineSemaphore));

    for (auto &swapchain : swapchainPool)
    {
        swapchain.Destroy();
    }

    commandBuffer.Reset();
    commandPool.Reset();
    swapchainPool[0].Destroy();

    ImmutableSampler.Reset();
    present.renderTargets.clear();
    renderPass.Reset();
    device.Reset();
    instance.DestroySurface(&surface);
}

void RenderContext::Prepare(size_t threadCount)
{
    present.renderTargets.resize(3);
    for (auto &r : present.renderTargets)
    {
        r = new RenderTarget;
    }

    UpdateSurface(surfaceExtent);

    SetupDescriptorSetLayout();
    __InitSyncObjects();
    Status = VK_SUCCESS;
}

Swapchain *RenderContext::UpdateSurface(VkExtent2D extent)
{
    VkSurfaceCapabilitiesKHR properties;
    Check(device->GetSurfaceCapabilities(&properties));

    if (Swapchain::IsValidExtent(properties.currentExtent))
    {
		extent = properties.currentExtent;
    }

    if (Swapchain::IsValidExtent(extent))
    {
		UpdateSwapchain(extent, regisry.preTransform);
        swapchain = &swapchainPool[0];
    }
    else
    {
		surfaceExtent = { 0, 0 };
        swapchain = nullptr;
    }

    return swapchain;
}

void RenderContext::UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
{
    auto swapchain = &swapchainPool[0];

    VkExtent2D swapchainExtent = swapchain->Get<VkExtent2D &>();
	if (extent.width != surfaceExtent.width || extent.height != surfaceExtent.height)
    {
		surfaceExtent = extent;
        swapchain->Invalidate(extent);

        auto &swapchainImages = swapchain->Get<Swapchain::Images &>();
        for (size_t i = 0; i < swapchainImages.size(); i++)
        {
            Image image = std::move(Image(
                device,
                swapchainImages[i],
                VkExtent3D{ extent.width, extent.height, 1 },
                swapchain->Get<VkFormat>(),
                swapchain->Get<VkImageUsageFlags>()
            ));

            present.renderTargets[i]->Invalidate(std::move(image), renderPass);
        }
    }

    regisry.preTransform = transform;
}

VkDescriptorSetLayout RenderContext::DescriptorSetLayout{ VK_NULL_HANDLE };
URef<Sampler> RenderContext::ImmutableSampler;
void RenderContext::SetupDescriptorSetLayout()
{
    VkSamplerCreateInfo samplerInfo = {
        .sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter     = VK_FILTER_LINEAR,
        .minFilter     = VK_FILTER_LINEAR,
        .mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .maxAnisotropy = 1.0f,
        .minLod        = -1000,
        .maxLod        = 1000,
    };

    ImmutableSampler = new Sampler{ device, samplerInfo };
    VkSampler sampler = *ImmutableSampler;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
    bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount    = 1;
    bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = U32(bindings.size());
    info.pBindings    = bindings.data();
    Check(device->Create(&info, &DescriptorSetLayout));
}

void RenderContext::OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    UpdateSurface(VkExtent2D{ width, height });
}

void RenderContext::PrepareFrame()
{
    VkResult ret = VK_SUCCESS;

    if (swapchain)
    {
        ret = swapchain->AcquireNextImage(&swapchainIndex, semaphores[sync].acquiredImageReady, VK_NULL_HANDLE);
        if (ret != VK_ERROR_OUT_OF_DATE_KHR && ret != VK_SUBOPTIMAL_KHR)
        {
            Check(ret);
        }

        uint64_t completion = 0;
		Check(device->GetCompletion(timelineSemaphore, &completion));
		if (completion < syncValues[sync])
		{
			VkSemaphore semapohre = timelineSemaphore;
			Check(device->Wait(&semapohre, &syncValues[sync]));
		}
    }

    commandBuffer = commandPool->Allocate();
    commandBuffer->Begin();

    syncValues[sync] = lastSync;
}

void RenderContext::__InitSyncObjects()
{
    for (int i = 0; i < FrameSize(); i++)
    {
        Check(device->AllocateSemaphore(&semaphores[i].acquiredImageReady));
        Check(device->AllocateSemaphore(&semaphores[i].renderComplete));
    }

    Check(device->AllocateSemaphore(&timelineSemaphore));
}

void RenderContext::SubmitFrame()
{
    PresentSubmitter submitter{};
    submitter.Wait(semaphores[sync].renderComplete);
    submitter.Present(*swapchain, swapchainIndex);

    VkResult ret = queue->Present(submitter);
    if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR)
    {
		UpdateSurface({ 0, 0 });
        return;
    }
    Check(ret);
}

void RenderContext::SwapBuffers()
{
    uint64_t lastWaitValue   = 0;
    uint64_t lastSignalValue = 0;

    commandBuffer->End();
    Submitter submitter{};
    TimelineSubmitter timelineSubmitter{};

    if (auto semaphore = device->GetTimelineSemaphore(); semaphore)
    {
        timelineSubmitter.Wait(semaphore->value);
        submitter.WaitSemaphore(*semaphore, VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    timelineSubmitter.Signal(++commandBuffer->semaphore.value);
    submitter.SignalSemaphore(commandBuffer->semaphore);
    timelineSubmitter.Signal(++syncValues[sync]);
	submitter.SignalSemaphore(timelineSemaphore);

    if (swapchain)
    {
        timelineSubmitter.Wait(0);
        submitter.WaitSemaphore(semaphores[sync].acquiredImageReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		timelineSubmitter.Signal(0);
		submitter.SignalSemaphore(semaphores[sync].renderComplete);
    }

    submitter.Execute(commandBuffer);
	commandPool->Free(commandBuffer);
	commandBuffer = nullptr;

    submitter.Trampoline(timelineSubmitter);
    queue->Submit(submitter, nullptr);

    if (swapchain)
    {
		SubmitFrame();
    }

    lastSync = syncValues[sync];
    SLROTATE(sync, FrameSize());

    device->DestroyObjects();
}

void RenderContext::Draw(GraphicsPipeline::Super *superPipeline)
{
    Submit([&](CommandBuffer *cmdbuf) {
        auto pipeline = dynamic_cast<GraphicsPipeline *>(superPipeline);

        VkDescriptorSet descriptorSets[] = {pipeline->GetDescriptorSet()};
        cmdbuf->BindDescriptorSets(
            GraphicsPipeline::BindPoint,
            pipeline->Layout(),
            0, 1,
            descriptorSets,
            0, 0);

        cmdbuf->BindPipeline(
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            *pipeline);

        Ref<Buffer> buffer = pipeline->Get<Buffer::Type::Vertex>();
        VkBuffer vertex = *buffer;
        VkDeviceSize offsets = buffer->Offset();
        cmdbuf->BindVertexBuffers(
            0, 1,
            &vertex,
            &offsets);

        buffer = pipeline->Get<Buffer::Type::Index>();
        cmdbuf->BindIndexBuffer(buffer);

        cmdbuf->DrawIndexed(
            pipeline->ElementCount,
            1, 0, 0, 0);
    });
}

DescriptorBuffer *RenderContext::CreateImageDescriptor(uint32_t count)
{
    auto ret = new DescriptorBuffer;
    ret->Request<ImageDescriptor>(count);

    return ret;
}

DescriptorBuffer *RenderContext::CreateBufferDescriptor(uint32_t count)
{
    auto ret = new DescriptorBuffer;
    ret->Request<BufferDescriptor>(count);

    return ret;
}

SuperShader *RenderContext::CreateShader(const std::string &filepath, Shader::Type type)
{
    return new Shader{device, filepath, type};
}

SuperGraphicsPipeline *RenderContext::CreateGraphicsPipeline(Ref<Shader::Super> shader)
{
    return new GraphicsPipeline{ device, shader };
}

SuperComputePipeline *RenderContext::CreateComputePipeline(Shader::Super *shader)
{
    return new ComputePipeline{device, shader};
}

SuperTexture *RenderContext::CreateTexture(const std::string &filepath, const Texture::Description &description)
{
    return new Texture{device, filepath, description};
}

SuperTexture *RenderContext::CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description)
{
    return new Texture{device, width, height, data, description};
}

SuperTextureCube *RenderContext::CreateTextureCube(uint32_t width, uint32_t height, const Texture::Description &description)
{
    return new TextureCube{device, width, height, description};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, const void *data, Buffer::Type type)
{
    return new Buffer{device, size, data, type};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, Buffer::Type type)
{
    return new Buffer{device, size, type};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, uint32_t binding)
{
    return new Buffer{device, size, binding};
}

SuperRenderTarget *RenderContext::CreateRenderTarget(const RenderTarget::Description &description)
{
    return new RenderTarget{device, description};
}

AccelerationStructure *RenderContext::CreateAccelerationStructure(const SuperBuffer *pVertexBuffer, const InputElementDescription &desc, const SuperBuffer *pIndexBuffer, const SuperBuffer *pTranformBuffer)
{
    return new AccelerationStructure{ device, (Buffer *)pVertexBuffer, desc, (Buffer *)pIndexBuffer, (Buffer *)pTranformBuffer };
}

SuperGuiLayer *RenderContext::CreateGuiLayer()
{
    return new GuiLayer{ this };
}

void RenderContext::PushConstant(SuperGraphicsPipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset)
{
    __PushConstant(dynamic_cast<Pipeline *>(pipeline), stage, size, data, offset);
}

void RenderContext::PushConstant(SuperComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset)
{
    __PushConstant(dynamic_cast<Pipeline *>(pipeline), Shader::Stage::Compute, size, data, offset);
}

void RenderContext::__PushConstant(Pipeline *pipeline, Shader::Stage stage, uint32_t size, const void * data, uint32_t offset)
{
    Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->PushConstants(
            pipeline->Layout(),
            stage,
            offset,
            size,
            data
            );
        });
}

void RenderContext::Dispatch(SuperComputePipeline *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
    Submit([&](CommandBuffer *cmdbuf) {
        auto pipeline = dynamic_cast<ComputePipeline *>(superPipeline);
        pipeline->Dispatch(cmdbuf, nGroupX, nGroupY, nGroupZ);
    });
}

void RenderContext::Blit(SuperTexture *superTexture)
{
	Submit([&](CommandBuffer *cmdbuf) {
		auto texture = dynamic_cast<Texture *>(superTexture);
		texture->Blit(cmdbuf);
	});
}

void RenderContext::Begin(SuperRenderTarget *iRenderTarget)
{
    auto renderTarget = dynamic_cast<RenderTarget *>(iRenderTarget);
    Submit([&](CommandBuffer *cmdbuf) {
        auto &desc = renderTarget->Desc();

        VkRenderPassBeginInfo beginInfo = {
            .sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext                    = nullptr,
            .renderPass               = renderTarget->Get<RenderPass>(),
            .framebuffer              = renderTarget->Get<Framebuffer>(),
            .renderArea               = { .offset = { 0, 0 }, .extent = { desc.Width, desc.Height}},
            .clearValueCount          = renderTarget->ColorAttachmentCount() + 1,
            .pClearValues             = rcast<VkClearValue*>(&renderTarget->clearValues),
        };

        cmdbuf->BeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        cmdbuf->SetViewport(ncast<float>(desc.Width), ncast<float>(desc.Height));
        cmdbuf->SetScissor(desc.Width, desc.Height);
    });
}

void RenderContext::End()
{
    Submit([&](CommandBuffer *cmdbuf) {
        cmdbuf->EndRenderPass();
    });
}

}
}
