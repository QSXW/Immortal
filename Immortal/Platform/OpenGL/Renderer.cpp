#include "impch.h"
#include "Renderer.h"

#include "Common.h"
#include <GLFW/glfw3.h>

#include "RenderTarget.h"
#include "Pipeline.h"
#include "Descriptor.h"

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
        LOG::ERR(message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        LOG::WARN(message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
    default:
        LOG::INFO(message);
        break;
    }
}

void Renderer::Setup()
{
#ifdef SLDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

    // glEnable(GL_FRAMEBUFFER_SRGB);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_STENCIL_TEST);
}

OpenGL::Renderer::Renderer(SuperRenderContext *c) :
    context{ dcast<RenderContext *>(c) }
{

}

void Renderer::OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const Vector::Vector4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::EnableDepthTest()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DisableDepthTest()
{
    glDisable(GL_DEPTH_TEST);
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::SwapBuffers()
{
    glfwSwapBuffers(context->Handle());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::Draw(Pipeline::Super *superPipeline)
{
   auto pipeline = dynamic_cast<Pipeline *>(superPipeline);
   pipeline->Draw();
}

void Renderer::Begin(std::shared_ptr<RenderTarget::Super> &superRenderTarget)
{
    auto target = std::dynamic_pointer_cast<RenderTarget>(superRenderTarget);
    target->Map();
}

void Renderer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Descriptor::Super *Renderer::CreateImageDescriptor(uint32_t count)
{
    auto descriptor = new Descriptor[count];
    THROWIF(!descriptor, SError::OutOfMemory);

    return descriptor;
}

}
}
