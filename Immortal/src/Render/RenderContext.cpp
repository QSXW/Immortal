#include "impch.h"
#include "RenderContext.h"

#include "Render.h"
#include "Platform/OpenGL/RenderContext.h"
#include "Platform/Vulkan/RenderContext.h"
#include "Platform/D3D12/RenderContext.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/Vulkan/Shader.h"

namespace Immortal
{

std::unique_ptr<RenderContext> RenderContext::Create(Description &desc)
{
    switch (Render::API)
    {
    case Render::Type::OpenGL:
        return std::make_unique<OpenGL::RenderContext>(desc);

    case Render::Type::Vulkan:
        return std::make_unique<Vulkan::RenderContext>(desc);

    case Render::Type::D3D12:
        return std::make_unique<D3D12::RenderContext>(desc);

    default:
        LOG::ERR("Not support api");
        return nullptr;
    }
}

std::shared_ptr<Shader> RenderContext::CreateShader(const std::string &filename, Shader::Type type)
{
    if (Render::API == Render::Type::OpenGL)
    {
        return std::make_shared<OpenGL::Shader>(filename);
    }
    if (Render::API == Render::Type::Vulkan)
    {
        auto context = dcast<Vulkan::RenderContext*>(this);
        return std::make_shared<Vulkan::Shader>(&(context->Get<Vulkan::Device>()), filename, type);
    }
    if (Render::API == Render::Type::D3D12)
    {
        return nullptr;
    }
    return nullptr;
}

}
