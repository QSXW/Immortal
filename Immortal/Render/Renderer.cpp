#include "impch.h"
#include "Renderer.h"

#include "Render.h"
#include "Platform/Vulkan/Renderer.h"
#include "Platform/OpenGl/Renderer.h"
#include "Platform/D3D12/Renderer.h"

namespace Immortal
{

std::unique_ptr<Renderer> Renderer::Create(RenderContext *context)
{
    if (Render::API == Render::Type::Vulkan)
    {
        return std::make_unique<Vulkan::Renderer>(context);
    }
    if (Render::API == Render::Type::OpenGL)
    {
         return std::make_unique<OpenGL::Renderer>(context);
    }
    if (Render::API == Render::Type::D3D12)
    {
        return std::make_unique<D3D12::Renderer>(context);
    }

    LOG::ERR("Renderer is not initialized. Forgot to set the Render API.");
    return nullptr;
}

}
