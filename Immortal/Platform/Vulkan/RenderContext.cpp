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
#if defined( _WIN32 )
    { "VK_KHR_win32_surface", false },
#elif defined(__linux__)
    { "VK_KHR_xcb_surface",   false },
#endif
#if defined(__APPLE__)
    { "VK_EXT_metal_surface",           false },
    { "VK_KHR_portability_enumeration", false },
#endif
};

std::unordered_map<const char *, bool> RenderContext::DeviceExtensions{
    { VK_KHR_SWAPCHAIN_EXTENSION_NAME,          false },
    { VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, false },
#ifdef __APPLE__
    { VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, false },
#endif
};

static std::vector<const char *> ValidationLayers = {
#ifdef _DEBUG
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2",
#endif
};

static Swapchain PrimarySwapchain;

RenderContext::RenderContext(const RenderContext::Description &desc) :
    window{ desc.window }
{
    instance = new Instance{ "Immortal Graphics API", InstanceExtensions, ValidationLayers};
    if (!instance)
    {
        LOG::ERR("Vulkan Not Supported!");
        return;
    }
    CreateSurface();

    if (instance->IsEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
    {
        AddDeviceExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
    }

    auto physicalDevice = instance->SuitablePhysicalDevice(desc.deviceId);
    physicalDevice->Activate(PhysicalDevice::Feature::SamplerAnisotropy);
    physicalDevice->Activate(PhysicalDevice::Feature::RobustBufferAccess);
    physicalDevice->Activate(PhysicalDevice::Feature::IndependentBlend);

    device = new Device{ physicalDevice, surface, DeviceExtensions };
    queue  = device->SuitableGraphicsQueuePtr();

	surfaceExtent.width  = desc.width;
	surfaceExtent.height = desc.height;
    if (surface != VK_NULL_HANDLE)
    {
		VkFormat depthFormat = physicalDevice->GetSuitableDepthFormat();
		swapchainPool[0] = std::move(Swapchain{ swapchainPool[0], device });
		renderPass = new RenderPass{ device, swapchainPool[0].Get<VkFormat>(), depthFormat };
        Prepare();
    }
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

    for (auto &swapchain : swapchainPool)
    {
		swapchain.Destroy();
    }

	swapchainPool[0].Destroy();

	semaphorePool.Reset();
    ImmutableSampler.Reset();
	present.renderTargets.clear();
	renderPass.Reset();
    device.Reset();
    instance->DestroySurfaceKHR(surface, nullptr);
}

void RenderContext::CreateSurface()
{
    Check(instance->CreateSurface(window, &surface));
}

void RenderContext::Prepare(size_t threadCount)
{
    device->Wait();
	present.renderTargets.resize(3);
	for (auto &r : present.renderTargets)
	{
		r = new RenderTarget;
	}

	UpdateSurface();

    SetupDescriptorSetLayout();
    __InitSyncObjects();
    Status = VK_SUCCESS;
}

Swapchain *RenderContext::UpdateSurface()
{
    VkSurfaceCapabilitiesKHR properties;
    Check(device->GetSurfaceCapabilities(&properties));

    if (properties.currentExtent.width == 0xFFFFFFFF || (properties.currentExtent.width <= 0 || properties.currentExtent.height <= 0))
    {
        surfaceExtent = properties.currentExtent;
		swapchain = nullptr;
    }
    if (properties.currentExtent.width != surfaceExtent.width || properties.currentExtent.height != surfaceExtent.height)
    {
        surfaceExtent = properties.currentExtent;
        UpdateSwapchain(surfaceExtent, regisry.preTransform);
		swapchain = &swapchainPool[0];
    }

    return swapchain;
}

void RenderContext::UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
{
	auto swapchain = &swapchainPool[0];

	swapchain->Invalidate(extent);
    regisry.preTransform = transform;

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
	__Resize();
}

void RenderContext::PrepareFrame()
{
	VkResult ret = VK_SUCCESS;
	if (!swapchain)
	{
		VkSurfaceCapabilitiesKHR properties;
		Check(device->GetSurfaceCapabilities(&properties));
		if (properties.currentExtent.width > 0 && properties.currentExtent.height > 0)
		{
			__Resize();
		}
	}

	if (swapchain)
	{
		ret = swapchain->AcquireNextImage(&swapchainIndex, semaphores[sync].acquiredImageReady, VK_NULL_HANDLE);
		if (ret != VK_ERROR_OUT_OF_DATE_KHR && ret != VK_SUBOPTIMAL_KHR)
		{
			Check(ret);
		}
	}

#ifdef VK_SYNC_TIMELINE_SEMAPHORE
#ifndef __linux__
	uint64_t completion = 0;
	Check(device->GetCompletion(timelineSemaphore, &completion));
	if (completion < syncValues[sync])
	{
		Check(device->Wait(&timelineSemaphore, &syncValues[sync]));
	}
#endif
#endif

	syncValues[sync] = lastSync;

#ifdef VK_SYNC_FENCE
#ifndef __linux__
	if (fences[sync] != VK_NULL_HANDLE)
	{
		device->WaitAndReset(&fences[sync]);
	}
	else
	{
		fences[sync] = device->RequestFence();
	}
#endif
#endif
}

void RenderContext::__Resize()
{
	UpdateSurface();
}

void RenderContext::__InitSyncObjects()
{
	semaphorePool = new SemaphorePool(device);
	for (int i = 0; i < FrameSize(); i++)
	{
		semaphores[i].acquiredImageReady = semaphorePool->Request();
		semaphores[i].renderComplete = semaphorePool->Request();
	}

	timelineSemaphore = semaphorePool->Request(0);
}

void RenderContext::__SubmitFrame()
{
	PresentSubmitter submitter{};
	submitter.Wait(semaphores[sync].renderComplete);
	submitter.Present(*swapchain, swapchainIndex);

	VkResult ret = queue->Present(submitter);
	if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR)
	{
		__Resize();
		return;
	}
	Check(ret);
}

void RenderContext::SwapBuffers()
{
	uint64_t lastWaitValue   = 0;
	uint64_t lastSignalValue = 0;

	if (device->IsCommandBufferRecorded<Queue::Type::Transfer>())
	{
		lastSignalValue = ++syncValues[sync];
		auto &commandBuffers = device->GetCommandBuffers<Queue::Type::Transfer>({ timelineSemaphore, lastSignalValue });
		TimelineSubmitter timelineSubmitter{};
		timelineSubmitter.Signal(lastSignalValue);

		Submitter submitter{};
		submitter.SignalSemaphore(timelineSemaphore);
		submitter.Execute(commandBuffers);
		submitter.Trampoline(timelineSubmitter);

		auto &transferQueue = device->FindQueueByType(Queue::Type::Transfer, device->QueueFailyIndex(Queue::Type::Transfer));
		transferQueue.Submit(submitter, nullptr);
	}

	if (device->IsCommandBufferRecorded<Queue::Type::Compute>())
	{
		lastWaitValue = lastSignalValue;
		lastSignalValue = ++syncValues[sync];
		auto &commandBuffers = device->GetCommandBuffers<Queue::Type::Compute>({ timelineSemaphore, lastSignalValue });

		TimelineSubmitter timelineSubmitter{};
		timelineSubmitter.Wait(lastWaitValue);
		timelineSubmitter.Signal(lastSignalValue);

		Submitter submitter{};
		submitter.WaitSemaphore(timelineSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		submitter.SignalSemaphore(timelineSemaphore);
		submitter.Execute(commandBuffers);
		submitter.Trampoline(timelineSubmitter);

		auto &computeQueue = device->FindQueueByType(Queue::Type::Compute, device->QueueFailyIndex(Queue::Type::Compute));
		computeQueue.Submit(submitter, nullptr);
	}

	lastWaitValue = lastSignalValue;
	lastSignalValue = ++syncValues[sync];

	TimelineSubmitter timelineSubmitter{};
	timelineSubmitter.Wait(lastWaitValue);
	timelineSubmitter.Signal(lastSignalValue);

	Submitter submitter{};
	submitter.WaitSemaphore(timelineSemaphore, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	submitter.SignalSemaphore(timelineSemaphore);
	submitter.Execute(device->GetCommandBuffers<Queue::Type::Graphics>({timelineSemaphore, lastSignalValue}));

	if (swapchain)
	{
		timelineSubmitter.Wait(0);
		timelineSubmitter.Signal(0);

		submitter.WaitSemaphore(semaphores[sync].acquiredImageReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		submitter.SignalSemaphore(semaphores[sync].renderComplete);
	}

	submitter.Trampoline(timelineSubmitter);
	queue->Submit(submitter, fences[sync]);

#ifdef __linux__
	// If the resize event triggered on ubuntu, the device->wait(fence) will get hang.
	// Not sure what exactly cause the problem at the present time.
	queue->WaitIdle();
#endif

	if (swapchain)
	{
		__SubmitFrame();
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
