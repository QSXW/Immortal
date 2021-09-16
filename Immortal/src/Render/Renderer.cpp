#include "impch.h"
#include "Renderer.h"

#include "Render.h"
#include "Platform/Vulkan/Renderer.h"
#include "Platform/OpenGl/Renderer.h"

namespace Immortal
{
static inline const char *Sringify(Render::Type type)
{
    switch (type)
    {
#define CASE(x) case Render::Type::x: return #x;
        CASE(None);
        CASE(Vulkan);
        CASE(D3D12);
#undef CASE
    default: return "Unknown";
    }
}

std::unique_ptr<Renderer> Renderer::Create(RenderContext *context)
{
    LOG::INFO("Initialize Renderer with API => {0}", Sringify(Render::API));
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
        return nullptr;
    }
}
}
