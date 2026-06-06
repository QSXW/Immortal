#include "Device.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Shader.h"
#include "Pipeline.h"
#include "GPUEvent.h"
#include "Texture.h"
#include "Sampler.h"
#include "Types.h"

namespace Immortal
{
namespace Metal
{

Device::Device() :
	physicalDevice{},
	Handle<MTL::Device>{},
	queues{}
{

}

Device::Device(PhysicalDevice *phsicalDevice) :
    physicalDevice{ phsicalDevice },
	Handle<MTL::Device>{},
	queues{}
{
	handle = MTL::CreateSystemDefaultDevice();
	queues[QueueType::Graphics] = handle->newCommandQueue();
    queues[QueueType::None    ] = handle->newCommandQueue();
}

Device::~Device()
{
	if (handle)
	{
		for (auto &[type, queue]: queues)
		{
			queue->release();
		}
	}
}

Anonymous Device::GetBackendHandle() const
{
	return (void *)handle;
}

BackendAPI Device::GetBackendAPI()
{
	return BackendAPI::Metal;
}

SuperSwapchain *Device::CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode)
{
	return new Swapchain{ this, InterpretAs<Queue>(queue), window, format, bufferCount, mode };
}

SuperQueue *Device::CreateQueue(QueueType type, QueuePriority priority)
{
	MTL::CommandQueue *queue = GetCommandQueue(type);
	return new Queue{ this, queue };
}

SuperCommandBuffer *Device::CreateCommandBuffer(QueueType type)
{
	MTL::CommandQueue *queue = GetCommandQueue(type);
	return new CommandBuffer{ this, queue };
}

SuperSampler *Device::CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	return new Sampler{ this, filter, addressMode, compareOperation, minLod, maxLod };
}

SuperShader *Device::CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint)
{
	return new Shader{ this, name, stage, source, entryPoint };
}

SuperGraphicsPipeline *Device::CreateGraphicsPipeline()
{
	return new GraphicsPipeline{ this };
}

SuperComputePipeline *Device::CreateComputePipeline(SuperShader *shader)
{
	return new ComputePipeline{ this, shader };
}

SuperTexture *Device::CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type)
{
	return new Texture{ this, format, width, height, mipLevels, arrayLayers, type };
}

SuperBuffer *Device::CreateBuffer(size_t size, BufferType type)
{
	return new Buffer{ this, type, size, nullptr };
}

SuperDescriptorSet *Device::CreateDescriptorSet(SuperPipeline *pipeline)
{
	return new DescriptorSet{ this, InterpretAs<Pipeline>(pipeline)};
}

SuperGPUEvent *Device::CreateGPUEvent(const std::string &name)
{
	return new GPUEvent{ this };
}

SuperRenderTarget *Device::CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat)
{
	return nullptr;
}

MTL::CommandQueue *Device::GetCommandQueue(QueueType type)
{
	MTL::CommandQueue *queue = nullptr;
	auto it = queues.find(type);
	if (it != queues.end())
	{
		queue = it->second;
	}
	else
	{
		queue = handle->newCommandQueue();
		queues[type] = queue;
	}

	return queue;
}

}
}
