#include "RenderContext.h"

#include "Render.h"
#include "Platform/OpenGL/RenderContext.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/D3D11/RenderContext.h"
#include "Platform/D3D12/RenderContext.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/Vulkan/Shader.h"

namespace Immortal
{

RenderContext *RenderContext::CreateInstance(const Description &desc)
{
    switch (Render::API)
    {
    case Render::Type::OpenGL:
        return new OpenGL::RenderContext(desc);

    case Render::Type::Vulkan:
        return new Vulkan::RenderContext(desc);

    case Render::Type::D3D11:
		return new D3D11::RenderContext(desc);

    case Render::Type::D3D12:
        return new D3D12::RenderContext(desc);

    default:
        throw Exception{ "Not support api" };
        return nullptr;
    }
}

}
