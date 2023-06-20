#include "Render.h"
#include "Render2D.h"
#include "Framework/Timer.h"

namespace Immortal
{

Ref<ComputePipeline> Pipelines::ColorSpace;
Ref<ComputePipeline> Pipelines::ColorSpaceNV122RGBA8;

RenderContext *Render::renderContext;

Render::Data Render::data{};

struct ShaderProperty
{
	uint32_t API;

	Shader::Type Type;
};

std::unordered_map<std::string, Ref<Shader>> shaderData;

const std::unordered_map <std::string, ShaderProperty> shaderProperties = {
    { "AnimatedBasic3D",        { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics }},
    { "Basic",                  { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics }},
    { "Basic3D",                { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics }},
    { "Texture",                { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics }},
    { "Render2D",               { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Graphics }},
    { "Outline",                { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics }},
    { "Skybox",                 { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Graphics }},
    { "PhysicalBasedRendering", { U32(Render::Type::Vulkan                                           ), Shader::Type::Graphics }},
    { "ColorMixing",            { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  }},
    { "color_space_yuv2rgba8",  { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  }},
    { "color_space_nv122rgba8", { U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D), Shader::Type::Compute  }},
    { "GaussianBlur",           { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  }},
    { "SimpleBlur",             { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  }},
    { "Equirect2Cube",          { U32(Render::Type::Vulkan | Render::Type::OpenGL                    ), Shader::Type::Compute  }},
};

Ref<Shader> Render::GetShader(const std::string &name)
{
	if (auto it = shaderData.find(name); it != shaderData.end())
    {
        return it->second;
    }
	else if (auto it = shaderProperties.find(name); it != shaderProperties.end())
    {
		auto &[name, shaderProperty] = *it;
        if ((Type)shaderProperty.API & API)
        {
			auto asset = API & Type::D3D ? 1 : 0;
			Ref<Shader> shader = Create<Shader>(std::string{AssetsPathes[asset]} + name, shaderProperty.Type);
			shaderData.insert({ name, shader });
			return shader;
        }
    }

    LOG::WARN("Request Shader {} Not Found!", name);
    return nullptr;
}

void Render::Setup(RenderContext *context)
{
    LOG::INFO("Initialize Renderer With Graphics API: {}", Sringify(Render::API));
	renderContext = context;

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
	shaderData.clear();

    Render2D::Release();
}

}
