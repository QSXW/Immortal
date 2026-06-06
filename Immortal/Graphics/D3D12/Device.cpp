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

D3D12_COMMAND_LIST_TYPE CAST(QueueType type)
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

void MessageCallbackFunc(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void *pContext)
{
	switch (severity)
	{
		case D3D12_MESSAGE_SEVERITY_WARNING:
			LOG::WARN("{}", pDescription);
			break;

		case D3D12_MESSAGE_SEVERITY_INFO:
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
			LOG::INFO("{}", pDescription);
			break;

		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
		case D3D12_MESSAGE_SEVERITY_ERROR:
	    default:
			LOG::ERR("{}", pDescription);
	}
}

Device::Device(PhysicalDevice *phsicalDevice) :
    physicalDevice{ phsicalDevice },
    descriptorPools{},
    shaderVisibleDescriptorPools{}
{
	Instance *instance = InterpretAs<Instance>(phsicalDevice->GetInstance());

    Check(instance->D3D12CreateDevice(
		*physicalDevice,
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&handle))
	);

#ifdef _DEBUG
	HRESULT hr = handle->QueryInterface<ID3D12InfoQueue1>(&infoQueue);
	if (SUCCEEDED(hr) && infoQueue)
	{
		DWORD callbackCookie = 0;
		hr = infoQueue->RegisterMessageCallback(&MessageCallbackFunc, D3D12_MESSAGE_CALLBACK_FLAG_NONE, (void *) this, &callbackCookie);
		if (FAILED(hr) || !callbackCookie)
		{
			LOG::ERR("Failed to register message callback!");
		}
	}
#endif

	CheckExtendFeatures();
}

Device::~Device()
{
	samplers.nearest.Reset();
	samplers.linear.Reset();
	pipelines.clear();

	for (uint32_t i = 0; i < SL_ARRAY_LENGTH(descriptorPools); i++)
	{
		for (size_t j = 0; j < SL_ARRAY_LENGTH(descriptorPools[i]); j++)
		{
			if (descriptorPools[i][j])
			{
				descriptorPools[i][j].Reset();
			}
		}
	}

	for (uint32_t i = 0; i < SL_ARRAY_LENGTH(shaderVisibleDescriptorPools); i++)
	{
		for (size_t j = 0; j < SL_ARRAY_LENGTH(shaderVisibleDescriptorPools[i]); j++)
		{
			if (shaderVisibleDescriptorPools[i][j])
			{
				descriptorPools[i][j].Reset();
			}
		}
	}

	infoQueue.Reset();
    handle.Reset();
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
	return new Queue{ this, type, priority };
}

SuperCommandBuffer *Device::CreateCommandBuffer(QueueType type)
{
	return new CommandBuffer{ this, CAST(type) };
}

SuperSampler *Device::CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	return new Sampler{ this, filter, addressMode, compareOperation, minLod, maxLod };
}

SuperShader *Device::CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint, const ShaderMacro *pMacro, uint32_t numMacro)
{
	return new Shader{ name, stage, source, entryPoint, pMacro, numMacro };
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

SuperBuffer *Device::CreateBuffer(BufferType type, size_t size)
{
	return new Buffer{ this, type, size };
}

SuperBuffer *Device::CreateBuffer(BufferType type, size_t size, MemoryType memoryType, uint32_t byteStride)
{
	return new Buffer{ this, type, size, memoryType, byteStride };
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
	return new RenderTarget{ this, width, height, pColorAttachmentFormats, colorAttachmentCount, depthAttachmentFormat };
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

bool Device::CheckExtendFeatures()
{
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_5};
	if (FAILED(handle->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
	{
		LOG::ERR("Shader Model 6.5 is not supported");
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
	if (FAILED(handle->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features))) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
	{
		LOG::ERR("Mesh Shaders aren't supported!");
		return false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS21 options;
	if (FAILED(handle->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options, sizeof(options))) ||
		options.WorkGraphsTier == D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED)
	{
		LOG::ERR("Device does not report support for work graphs.");
		return false;
	}

	return true;
}

void Device::CreateSampler(const D3D12_SAMPLER_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptor)
{
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
		{ "GenerateMipMaps",     { "Assets/Shaders/hlsl/generatemipmaps.hlsl",     ShaderStage::Compute, "GenerateMipMaps"    } },
		{ "GenerateMipMapsCube", {"Assets/Shaders/hlsl/generatemipmaps_cube.hlsl", ShaderStage::Compute, "GenerateMipMapsCube"} }
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

	std::unique_lock lock{ pipelineMutex };
	pipelines[name] = std::move(pipeline);

	return pipelines[name];
}

Sampler *Device::GetSampler(Filter filter)
{
	if (filter == Filter::Nearest)
	{
		if (!samplers.nearest)
		{
			samplers.nearest = new Sampler{ this, Filter::Nearest, AddressMode::Clamp };
		}
		return samplers.nearest;
	}
	if (filter == Filter::Linear || filter == Filter::Bilinear)
	{
		if (!samplers.linear)
		{
			samplers.linear = new Sampler{this, Filter::Linear, AddressMode::Clamp };
		}
		return samplers.linear;
	}
	return nullptr;
}

Descriptor Device::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap **ppHeap, uint32_t descriptorCount)
{
	if (descriptorCount < SL_ARRAY_LENGTH(descriptorPools[type]))
	{
		std::lock_guard lock{ descritorHeapMutex };
		auto &allocator = descriptorPools[type][descriptorCount];
		if (!allocator)
		{
			allocator = new DescriptorPool{
				this,
				type,
				descriptorCount
			};
		}

		return allocator->AllocateWithMask(ppHeap, descriptorCount);
	}

	DescriptorHeap *descriptorHeap = new DescriptorHeap{ this, descriptorCount, type, D3D12_DESCRIPTOR_HEAP_FLAG_NONE };
	Descriptor descriptor = { descriptorHeap->GetCPUDescriptorHandle(), descriptorHeap->GetIncrementSize() };
	*ppHeap = descriptorHeap;

	return descriptor;
}

void Device::FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap *descriptorHeap, Descriptor descriptor, uint32_t descriptorCount)
{
	std::lock_guard lock{ descritorHeapMutex };
	auto &allocator = descriptorPools[type][descriptorCount];
	allocator->Free(descriptorHeap, descriptor, descriptorCount);
}

void Device::AllocateShaderVisibleDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap **ppHeap, ShaderVisibleDescriptor *pBaseDescriptor, uint32_t descriptorCount)
{
	if (descriptorCount < SL_ARRAY_LENGTH(shaderVisibleDescriptorPools[type]))
	{
		std::lock_guard lock{shaderVisibleDescriptorMutex};
		auto &allocator = shaderVisibleDescriptorPools[type][descriptorCount];
		if (!allocator)
		{
			allocator = new DescriptorPool{
				this,
				type,
				descriptorCount,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			};
		}

		pBaseDescriptor->descriptor = allocator->AllocateWithMask(ppHeap, descriptorCount);
		pBaseDescriptor->shaderVisibleDescriptor = (*ppHeap)->GetGPUDescriptorHandle();

		uint32_t increment = pBaseDescriptor->descriptor.GetIncrementSize();
		auto index = (pBaseDescriptor->descriptor.ptr - (*ppHeap)->GetCPUDescriptorHandle().ptr) / increment;
		pBaseDescriptor->shaderVisibleDescriptor.Offset(index, increment);

		return;
	}

	DescriptorHeap *descriptorHeap = new DescriptorHeap{this, descriptorCount, type, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};
	ShaderVisibleDescriptor descriptor = {
	    .descriptor = {descriptorHeap->GetCPUDescriptorHandle(), descriptorHeap->GetIncrementSize()},
	    .shaderVisibleDescriptor = descriptorHeap->GetGPUDescriptorHandle()};
	*pBaseDescriptor = descriptor;
	*ppHeap = descriptorHeap;
}

void Device::FreeShaderVisibleDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, DescriptorHeap *descriptorHeap, Descriptor descriptor, uint32_t descriptorCount)
{
	if (descriptorCount < SL_ARRAY_LENGTH(shaderVisibleDescriptorPools[type]))
	{
		std::lock_guard lock{shaderVisibleDescriptorMutex};
		auto &allocator = shaderVisibleDescriptorPools[type][descriptorCount];
		allocator->Free(descriptorHeap, descriptor, descriptorCount);
	}
	else
	{
		delete descriptorHeap;
	}
}

}
}
