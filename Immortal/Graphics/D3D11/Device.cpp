#include "Device.h"
#include "Swapchain.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "GPUEvent.h"
#include "Pipeline.h"
#include "Queue.h"
#include "RenderTarget.h"
#include "Sampler.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Texture.h"
#include "PhysicalDevice.h"
#include "Instance.h"

namespace Immortal
{
namespace D3D11
{

Device::Device(PhysicalDevice *physicalDevice) :
    handle{},
    physicalDevice{ physicalDevice },
    context{},
    featureLevel{}
{
	Instance *instance = InterpretAs<Instance>(physicalDevice->GetInstance());

	uint32_t deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> deviceContext;
	instance->D3D11CreateDevice(
		*physicalDevice,
	    D3D_DRIVER_TYPE_UNKNOWN,
		0,
	    deviceFlags,
	    FeatureLevels,
	    SL_ARRAY_LENGTH(FeatureLevels),
	    D3D11_SDK_VERSION,
	    &device,
	    &featureLevel,
	    &deviceContext
	);

	device.As<ID3D11Device5>(&handle);
	deviceContext.As<ID3D11DeviceContext4>(&context);
}

Device::~Device()
{
	context.Reset();
	handle.Reset();
}

Anonymous Device::GetBackendHandle() const
{
	return (void *) handle.Get();
}

BackendAPI Device::GetBackendAPI()
{
	return BackendAPI::D3D11;
}

SuperQueue *Device::CreateQueue(QueueType type, QueuePriority priority)
{
	return new Queue{};
}

SuperCommandBuffer *Device::CreateCommandBuffer(QueueType type)
{
	return new CommandBuffer{ this };
}

SuperSwapchain *Device::CreateSwapchain(SuperQueue */*queue*/, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode)
{
	return new Swapchain{ this, window, format, bufferCount, mode };
}

SuperSampler *Device::CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	return new Sampler{ this, filter, addressMode, compareOperation, minLod, maxLod };
}

SuperShader *Device::CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint, const ShaderMacro *pMacro, uint32_t numMacro)
{
	return new Shader{ this, name, ShaderSourceType::HLSL, stage, source, entryPoint };
}

SuperGraphicsPipeline *Device::CreateGraphicsPipeline()
{
	return new GraphicsPipeline{ this };
}

SuperBuffer *Device::CreateBuffer(size_t size, BufferType type)
{
	return new Buffer{ this, size, type };
}

SuperTexture *Device::CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type)
{
	return new Texture{ this, format, width, height, mipLevels, arrayLayers, type };
}
    
SuperDescriptorSet *Device::CreateDescriptorSet(SuperPipeline *pipeline)
{
	return new DescriptorSet{ InterpretAs<Pipeline>(pipeline) };
}

SuperGPUEvent *Device::CreateGPUEvent(const std::string &name)
{
	return new GPUEvent{};
}

SuperRenderTarget *Device::CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat)
{
	return nullptr;
}

IDXGIFactory4 *Device::GetDXGIFactory() const
{
	Instance *instance = InterpretAs<Instance>(physicalDevice->GetInstance());
	return *instance;
}

IDXGIAdapter1 *Device::GetAdapter() const
{
	return *physicalDevice;
}

}
}
