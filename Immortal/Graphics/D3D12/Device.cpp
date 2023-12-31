#include "Device.h"
#include "CommandBuffer.h"
#include "PhysicalDevice.h"
#include "Buffer.h"
#include "Sampler.h"
#include "Event.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "RenderTarget.h"
#include "Swapchain.h"
#include "Texture.h"
#include "Shader.h"
#include "Pipeline.h"
#include "Instance.h"
#include <filesystem>
#include <dxgidebug.h>
#include <fstream>

namespace Immortal
{
namespace D3D12
{

static D3D12_COMMAND_LIST_TYPE CAST(QueueType type)
{
	switch (type)
	{
		case QueueType::Graphics:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case QueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case QueueType::Transfer:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		case QueueType::VideoDecoding:
			return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
		case QueueType::VideoEncoding:
			return D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
		default:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}
}

Device::Device(PhysicalDevice *phsicalDevice) :
    physicalDevice{ phsicalDevice }
{
	Instance *instance = InterpretAs<Instance>(phsicalDevice->GetInstance());

    Check(instance->D3D12CreateDevice(
		*physicalDevice,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&handle))
	);
}

Device::~Device()
{
	samplers.nearest.Reset();
	samplers.linear.Reset();
	pipelines.clear();

	for (uint32_t i = 0; i < SL_ARRAY_LENGTH(descriptorPools); i++)
	{
		descriptorPools[i].Reset();
		shaderVisibleDescriptorPools[i].Reset();
	}

    handle.Reset();

#ifdef _DEBUG
    HMODULE dxgidebug = LoadLibraryA("dxgidebug.dll");
	if (dxgidebug)
    {
        ComPtr<IDXGIDebug> dxgiDebug;
		auto __DXGIGetDebugInterface = (decltype(&DXGIGetDebugInterface))::GetProcAddress(dxgidebug, "DXGIGetDebugInterface");
        Check(__DXGIGetDebugInterface(IID_PPV_ARGS(&dxgiDebug)));
        Check(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));

		FreeLibrary(dxgidebug);
    }
#endif
}

Anonymous Device::GetBackendHandle() const
{
	return (void *)handle.Get();
}

BackendAPI Device::GetBackendAPI()
{
	return BackendAPI::D3D12;
}

SuperSwapchain *Device::CreateSwapchain(SuperQueue *_queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode)
{
	Queue *queue = InterpretAs<Queue>(_queue);
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {
	    .Width       = window->GetWidth(),
	    .Height      = window->GetHeight(),
	    .Format      = format,
	    .Stereo      = FALSE,
	    .SampleDesc  = {.Count = 1, .Quality = 0},
	    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
	    .BufferCount = bufferCount,
	    .Scaling     = DXGI_SCALING_STRETCH,
	    .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	    .AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
	    .Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
	};

	auto swapchain = new Swapchain{ this, queue, window, swapchainDesc, mode };

	return swapchain;
}

SuperQueue *Device::CreateQueue(QueueType type, QueuePriority priority)
{
	D3D12_COMMAND_QUEUE_DESC desc{
		.Type     = CAST(type),
		.Priority = (INT)priority,
		.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0,
	};

	return new Queue{ this, desc };
}

SuperCommandBuffer *Device::CreateCommandBuffer(QueueType type)
{
	return new CommandBuffer{ this, CAST(type) };
}

SuperSampler *Device::CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	return new Sampler{ this, filter, addressMode, compareOperation, minLod, maxLod };
}

SuperShader *Device::CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint)
{
	return new Shader{ name, stage, source, entryPoint };
}

SuperGraphicsPipeline *Device::CreateGraphicsPipeline()
{
	return new GraphicsPipeline{ this };
}

SuperComputePipeline *Device::CreateComputePipeline(SuperShader *shader)
{
	return new ComputePipeline{ this,  shader };
}

SuperTexture *Device::CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type)
{
	return new Texture{ this, format, width, height, mipLevels, arrayLayers, type };
}

SuperBuffer *Device::CreateBuffer(size_t size, BufferType type)
{
	return new Buffer{ this, type, size };
}

SuperDescriptorSet *Device::CreateDescriptorSet(SuperPipeline *pipeline)
{
	return new DescriptorSet{ this, InterpretAs<Pipeline>(pipeline) };
}

SuperGPUEvent *Device::CreateGPUEvent(const std::string &name)
{
	(void)name;
	return new GPUEvent{ this };
}

SuperRenderTarget *Device::CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat)
{
	return nullptr;
}

IDXGIAdapter1 *Device::GetAdapter() const
{
	return *physicalDevice;
}

IDXGIFactory4 *Device::GetDXGIFactory() const
{
	Instance *instance =  InterpretAs<Instance>(physicalDevice->GetInstance());
	return *instance;
}

void Device::CreateSampler(const D3D12_SAMPLER_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptor)
{
	*pDestDescriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	handle->CreateSampler(pDesc, *pDestDescriptor);
}

Pipeline *Device::GetPipeline(const std::string &name)
{
	struct PipelineCreateInfo
	{
		const char *path;
		ShaderStage stage;
		const char *enryPoint;
	};

	static const std::unordered_map<std::string, PipelineCreateInfo> pipelineCreateInfos = {
		{ "GenerateMipMaps", { "Assets/Shaders/hlsl/generatemipmaps.hlsl", ShaderStage::Compute, "GenerateMipMaps"} }
	};

	{
		std::shared_lock lock{ pipelineMutex };
		auto it = pipelines.find(name);
		if (it != pipelines.end())
		{
			return it->second;
		}
	}

	auto pipelineCreateInfoIt = pipelineCreateInfos.find(name);
	if (pipelineCreateInfoIt == pipelineCreateInfos.end())
	{
		return nullptr;
	}

	const auto &[shaderName, pipelineCreateInfo] = *pipelineCreateInfoIt;

	std::ifstream stream;
	stream.open(pipelineCreateInfo.path, std::ios::binary);
	if (!stream.is_open())
	{
		return nullptr;
	}

	size_t size = std::filesystem::file_size(pipelineCreateInfo.path);
	std::string shaderSource;
	shaderSource.resize(size);

	stream.read(shaderSource.data(), size);

	Shader shader{ shaderName, pipelineCreateInfo.stage, shaderSource, pipelineCreateInfo.enryPoint };
	URef<Pipeline> pipeline = new ComputePipeline{ this, &shader };

	std::shared_lock lock{ pipelineMutex };
	pipelines[name] = std::move(pipeline);

	return pipelines[name];
}

Sampler *Device::GetSampler(Filter filter)
{
	if (filter == Filter::Nearest)
	{
		if (!samplers.nearest)
		{
			samplers.nearest = new Sampler{ this, Filter::Nearest, AddressMode::Wrap };
		}
		return samplers.nearest;
	}
	if (filter == Filter::Linear)
	{
		if (!samplers.linear)
		{
			samplers.linear = new Sampler{this, Filter::Linear, AddressMode::Wrap};
		}
		return samplers.linear;
	}
	return nullptr;
}

Descriptor Device::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount)
{
	auto &allocator = descriptorPools[type];
	if (!allocator)
	{
		allocator = new DescriptorPool{ 
			this,
			type
		};
	}
	return allocator->Allocate(descriptorCount);
}

void Device::AllocateShaderVisibleDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap **ppHeap, ShaderVisibleDescriptor *pBaseDescriptor, uint32_t descriptorCount)
{
	auto &allocator = shaderVisibleDescriptorPools[type];
	if (!allocator)
	{
		allocator = new DescriptorPool{
			this,
		    type,
		    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		};
	}
	allocator->Allocate(ppHeap, pBaseDescriptor, descriptorCount);
}

void Device::Destory(ID3D12Resource *pResource)
{
	pResource->AddRef();
	std::lock_guard lock{ resourceMutex };
	resources.emplace_back(pResource);
}

}
}
