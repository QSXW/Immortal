#include "Device.h"
#include <GLFW/glfw3.h>

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Pipeline.h"
#include "Queue.h"
#include "RenderTarget.h"
#include "Sampler.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Texture.h"
#include "GPUEvent.h"
#include "DescriptorSet.h"
#include "Shared/Log.h"

namespace Immortal
{
namespace OpenGL
{

void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char *message, const void *userParam)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
		case GL_DEBUG_SEVERITY_MEDIUM:
			LOG::ERR("{}", message);
			break;

		case GL_DEBUG_SEVERITY_LOW:
			LOG::WARN("{}", message);
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
		default:
			LOG::INFO("{}", message);
			break;
	}
}

Device::Device() :
    loaded{}
{

}

Device::~Device()
{

}

Anonymous Device::GetBackendHandle() const
{
	return (void *)glfwGetCurrentContext();
}

BackendAPI Device::GetBackendAPI()
{
	return BackendAPI::OpenGL;
}

SuperQueue *Device::CreateQueue(QueueType type, QueuePriority priority)
{
	return new Queue{};
}

SuperCommandBuffer *Device::CreateCommandBuffer(QueueType type)
{
	return new CommandBuffer{};
}

SuperSwapchain *Device::CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode)
{
	if (!loaded)
	{
		glfwMakeContextCurrent((GLFWwindow *) window->GetBackendHandle());

		int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		SLASSERT(status && "Failed to initialize Glad!");

#ifdef _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
		loaded = true;
	}

	return new Swapchain{ window, format, bufferCount, mode };
}

SuperSampler *Device::CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod)
{
	return new Sampler{ filter, addressMode, compareOperation, minLod, maxLod };
}

SuperShader *Device::CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint, const ShaderMacro *pMacro, uint32_t numMacro)
{
	return new Shader{ name, stage, source, entryPoint };
}

SuperGraphicsPipeline *Device::CreateGraphicsPipeline()
{
	return new Pipeline{};
}

SuperBuffer *Device::CreateBuffer(size_t size, BufferType type)
{
	return new Buffer{ size, type };
}

SuperTexture *Device::CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type)
{
	return new Texture{ format, width, height, mipLevels, arrayLayers, type };
}

SuperDescriptorSet *Device::CreateDescriptorSet(SuperPipeline *pipeline)
{
	return new DescriptorSet{ InterpretAs<Pipeline>(pipeline) };
}

SuperGPUEvent *Device::CreateGPUEvent(const std::string &/*name*/)
{
	return new GPUEvent{};
}

SuperRenderTarget *Device::CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat)
{
	return new RenderTarget{ width, height, pColorAttachmentFormats, colorAttachmentCount, depthAttachmentFormat };
}

}
}
