#include "Render.h"
#include "Render2D.h"
#include "Framework/Timer.h"

namespace Immortal
{

Ref<ComputePipeline> Pipelines::ColorSpace;
Ref<ComputePipeline> Pipelines::ColorSpaceNV122RGBA8;

RenderContext *Render::renderContext;

Shader::Manager Render::ShaderManager;

Render::Data Render::data{};

const Shader::Properties Render::ShaderProperties[] = {
    { "AnimatedBasic3D",        U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics },
    { "Basic",                  U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics },
    { "Basic3D",                U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics },
    { "Texture",                U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics },
    { "Render2D",               U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics },
    { "Outline",                U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics },
    { "Skybox",                 U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics },
    { "PhysicalBasedRendering", U32(Render::Type::Vulkan                                           ), Shader::Type::Graphics },
    { "ColorMixing",            U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  },
    { "color_space_yuv2rgba8",  U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  },
    { "color_space_nv122rgba8", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  },
    { "GaussianBlur",           U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  },
    { "SimpleBlur",             U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  },
    { "Equirect2Cube",          U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  },
};

Ref<Shader> Render::GetShader(const std::string &name)
{
    if (auto it = ShaderManager.find(name); it != ShaderManager.end())
    {
        return it->second;
    }

    LOG::WARN("Request Shader {} Not Found!", name);
    return nullptr;
}

void Render::Setup(RenderContext *context)
{
    LOG::INFO("Initialize Renderer With Graphics API: {}", Sringify(Render::API));
	renderContext = context;

    {
        Profiler profiler{ "Loading Shader" };
        auto asset = API & Type::D3D ? 1 : 0;
        for (int i = 0; i < SL_ARRAY_LENGTH(ShaderProperties); i++)
        {
            if (ncast<Render::Type>(ShaderProperties[i].API) & API)
            {
                Ref<Shader> shader = Create<Shader>(std::string{ AssetsPathes[asset] } + ShaderProperties[i].Name, ShaderProperties[i].Type);
                ShaderManager.emplace(ShaderProperties[i].Name, std::move(shader));
            }
        }
    }

    Pipelines::ColorSpace = Create<ComputePipeline>(GetShader("color_space_yuv2rgba8"));
	Pipelines::ColorSpaceNV122RGBA8 = Create<ComputePipeline>(GetShader("color_space_nv122rgba8"));

    data.Target = Render::CreateRenderTarget({
        viewport,
        {
            { Format::RGBA8    },
            { Format::R32      },
            { Format::Depth32F }
        }
        });

    {
        constexpr uint32_t white        = 0xffffffff;
        constexpr uint32_t black        = 0x000000ff;
        constexpr uint32_t transparency = 0x00000000;
        constexpr uint32_t normal       = 0xffff7f7f;
        Texture::Description desc = { Format::RGBA8, Wrap::Repeat, Filter::Linear };

        data.Textures.White       = Render::Create<Texture>(1, 1, &white, desc       );
        data.Textures.Black       = Render::Create<Texture>(1, 1, &black, desc       );
        data.Textures.Transparent = Render::Create<Texture>(1, 1, &transparency, desc);
        data.Textures.Normal      = Render::Create<Texture>(1, 1, &normal,       desc);
    }
    Render2D::Setup();
}

void Render::Setup(Ref<RenderTarget> &renderTarget)
{
    Render2D::Setup(renderTarget);
}

void Render::Release()
{
    Pipelines::ColorSpace.Reset();
	Pipelines::ColorSpaceNV122RGBA8.Reset();

    data.Textures.White.Reset();
    data.Textures.Black.Reset();
    data.Textures.Transparent.Reset();
    data.Textures.Normal.Reset();
    data.Target.Reset();
    ShaderManager.clear();

    Render2D::Release();
}

}
