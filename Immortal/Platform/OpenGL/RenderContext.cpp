#include "RenderContext.h"

#include "Common.h"
#include <GLFW/glfw3.h>
#include "GuiLayer.h"

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

RenderContext::RenderContext(const Description &desc)
    : handle(static_cast<GLFWwindow *>(desc.WindowHandle->GetNativeWindow()))
{
    SLASSERT(handle && "Window Handle is null!");
    Setup();
}

void RenderContext::Setup()
{
    glfwMakeContextCurrent(handle);
    int status = gladLoadGLLoader(rcast<GLADloadproc>(glfwGetProcAddress));

    SLASSERT(status && "Failed to initialize Glad!");
    SLASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5) && "Immortal requires at least OpenGL version 4.5!");

    Super::UpdateMeta(rcast<const char *>(glGetString(GL_RENDERER)), rcast<const char *>(glGetString(GL_VERSION)), rcast<const char *>(glGetString(GL_VENDOR)));

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLMessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_STENCIL_TEST);
}

SuperGuiLayer *RenderContext::CreateGuiLayer()
{
    return new GuiLayer{ this };
}

void RenderContext::OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

void RenderContext::SwapBuffers()
{
	glfwSwapBuffers(handle);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

SuperTexture *RenderContext::CreateTexture(const std::string &filepath, const Texture::Description &description)
{
	return new Texture{filepath, description};
}

SuperTexture *RenderContext::CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description)
{
	return new Texture{width, height, data, description};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, uint32_t binding)
{
	return new UniformBuffer{U32(size), binding};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, const void *data, Buffer::Type type)
{
	return new Buffer{U32(size), data, type};
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, Buffer::Type type)
{
	return new Buffer{U32(size), type};
}

SuperGraphicsPipeline *RenderContext::CreateGraphicsPipeline(Ref<SuperShader> shader)
{
	return new Pipeline{shader};
}

SuperComputePipeline *RenderContext::CreateComputePipeline(Shader::Super *shader)
{
	return new Pipeline{shader};
}

SuperRenderTarget *RenderContext::CreateRenderTarget(const RenderTarget::Description &description)
{
	return new RenderTarget{description};
}

SuperShader *RenderContext::CreateShader(const std::string &filepath, Shader::Type type)
{
	return new Shader{filepath, type};
}

DescriptorBuffer *RenderContext::CreateImageDescriptor(uint32_t count)
{
	auto ret = new DescriptorBuffer;
	ret->Request<uint32_t>(count);
	return ret;
}
void RenderContext::Draw(SuperGraphicsPipeline *superPipeline)
{
	auto pipeline = dynamic_cast<Pipeline *>(superPipeline);
	pipeline->Draw();
}

void RenderContext::Begin(RenderTarget::Super *superRenderTarget)
{
	auto target = dynamic_cast<RenderTarget *>(superRenderTarget);
	target->Map();
}

void RenderContext::End()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderContext::PushConstant(ComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset)
{
	pipeline->PushConstant(size, data, offset);
}

}
}
