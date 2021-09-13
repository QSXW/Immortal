#include "impch.h"
#include "RenderContext.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderContext.h"
#include "Platform/Vulkan/RenderContext.h"

namespace Immortal
{

Unique<RenderContext> RenderContext::Create(Description &desc)
{
    switch (Renderer::API())
    {
    case RendererAPI::Type::OpenGL:
        return MakeUnique<OpenGLRenderContext>(desc);

    case RendererAPI::Type::VulKan:
        return MakeUnique<Vulkan::RenderContext>(desc);

    default:
        LOG::ERR("Not support api");
        return nullptr;
    }
}

}
