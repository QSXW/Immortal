#include "Renderer.h"

#include "Render.h"
#include "Platform/Vulkan/Renderer.h"
#include "Platform/OpenGL/Renderer.h"

#ifdef _MSC_VER
#include "Platform/D3D12/Renderer.h"
#endif

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
#ifdef _MSC_VER
    if (Render::API == Render::Type::D3D12)
    {
        return std::make_unique<D3D12::Renderer>(context);
    }
#endif

    LOG::ERR("Renderer is not initialized. Render API specified is not supported yet.");
    return nullptr;
}

}
