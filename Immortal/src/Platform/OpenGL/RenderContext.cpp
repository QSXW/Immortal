#include "impch.h"
#include "RenderContext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Immortal
{
namespace OpenGL
{
RenderContext::RenderContext(RenderContext::Description &desc)
    : handle(static_cast<GLFWwindow *>(desc.WindowHandle->GetNativeWindow()))
{
    SLASSERT(handle && "Window Handle is null!");
    INIT();
}

void RenderContext::INIT()
{
    glfwMakeContextCurrent(handle);
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    SLASSERT(status && "Failed to initialize Glad!");
    SLASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5) && "Immortal requires at least OpenGL version 4.5!");

    Super::UpdateMeta(rcast<const char *>(glGetString(GL_RENDERER)), rcast<const char *>(glGetString(GL_VERSION)), rcast<const char *>(glGetString(GL_VENDOR)));

    LOG::INFO("Renderer: {0}", graphicsRenderer.c_str());
    LOG::INFO("Vecdor: {0}", vendor.c_str());
    LOG::INFO("Version: {0}", driverVersion.c_str());
}

}
}
