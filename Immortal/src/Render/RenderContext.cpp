#include "impch.h"
#include "RenderContext.h"

#include "Render.h"
#include "Platform/OpenGL/OpenGLRenderContext.h"
#include "Platform/Vulkan/RenderContext.h"

namespace Immortal
{

Unique<RenderContext> RenderContext::Create(Description &desc)
{
    switch (Render::API)
    {
    case Render::Type::OpenGL:
        return std::make_unique<OpenGLRenderContext>(desc);

    case Render::Type::Vulkan:
        return std::make_unique<Vulkan::RenderContext>(desc);

    default:
        LOG::ERR("Not support api");
        return nullptr;
    }
}

}
