#include "impch.h"
#include "Render.h"
#include "Render2D.h"

namespace Immortal
{

std::unique_ptr<Renderer> Render::renderer;

std::vector<std::shared_ptr<Shader>> Render::ShaderContainer{};

Render::Scene Render::scene{};

Render::Data Render::data{};

const Shader::Properties Render::ShaderProperties[] = {
    {                  "Basic", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D12), Shader::Type::Graphics },
    {                "Texture", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D12), Shader::Type::Graphics },
    {               "Render2D", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D12), Shader::Type::Graphics },
    { "PhysicalBasedRendering", U32(Render::Type::Vulkan                                             ), Shader::Type::Graphics },
    {            "ColorMixing", U32(Render::Type::Vulkan | Render::Type::OpenGL                      ), Shader::Type::Compute  },
    {             "SimpleBlur", U32(Render::Type::Vulkan | Render::Type::OpenGL                      ), Shader::Type::Compute  },
    {                "Basic3D", U32(Render::Type::Vulkan | Render::Type::OpenGL                      ), Shader::Type::Graphics },
};

void Render::Setup(RenderContext *context)
{
    LOG::INFO("Initialize Renderer with API => {0}", Sringify(Render::API));
    renderer = Renderer::Create(context);
    renderer->Setup();

    {
        Profiler profiler{ "Loading Shader" };
        auto asset = API == Type::D3D12 ? 1 : 0;
        ShaderContainer.reserve(SL_ARRAY_LENGTH(ShaderProperties));
        for (int i = 0; i < SL_ARRAY_LENGTH(ShaderProperties); i++)
        {
            if (ncast<Render::Type>(ShaderProperties[i].API) & API)
            {
                auto &&shader = std::shared_ptr<Shader>{ Create<Shader>(std::string{ AssetsPathes[asset] } + ShaderProperties[i].Path, ShaderProperties[i].Type) };
                ShaderContainer.emplace_back(std::move(shader));
            }    
        }
    }

    data.Target.reset(Render::CreateRenderTarget({
        viewport,
        {
            { Format::RGBA8 },
            { Format::R32   },
            { Format::Depth }
        }
        }));

    {
        constexpr uint32_t white        = 0xffffffff;
        constexpr uint32_t black        = 0x000000ff;
        constexpr uint32_t transparency = 0x00000000;
        Texture::Description desc = { Format::RGBA8, Wrap::Repeat, Filter::Linear };

        data.WhiteTexture       = std::shared_ptr<Texture>{ Render::Create<Texture>(1, 1, &white, desc)        };
        data.BlackTexture       = std::shared_ptr<Texture>{ Render::Create<Texture>(1, 1, &black, desc)        };
        data.TransparentTexture = std::shared_ptr<Texture>{ Render::Create<Texture>(1, 1, &transparency, desc) };
    }
    Render2D::Setup();
}

void Render::Setup(const std::shared_ptr<RenderTarget> &renderTarget)
{
    Render2D::Setup(renderTarget);
}

}
