#pragma once

#include "Core.h"
#include "Render/RenderContext.h"

struct GLFWwindow;
namespace Immortal
{
namespace OpenGL
{

class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

public:
    RenderContext(SuperRenderContext::Description &desc);

    void Setup();

    GLFWwindow *Handle()
    {
        return handle;
    }

private:
    GLFWwindow *handle;
};

}
}
