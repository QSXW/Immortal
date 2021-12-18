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
    {    "Basic", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D12), Shader::Type::Graphics },
    {  "Texture", U32(Render::Type::Vulkan | Render::Type::OpenGL | Render::Type::D3D12), Shader::Type::Graphics },
    { "Render2D", U32(Render::Type::Vulkan | Render::Type::OpenGL                      ), Shader::Type::Graphics }
};

void Render::Setup(RenderContext *context)
{
    LOG::INFO("Initialize Renderer with API => {0}", Sringify(Render::API));
    renderer = Renderer::Create(context);
    renderer->Setup();

    {
        auto asset = API == Type::D3D12 ? 1 : 0;
        ShaderContainer.reserve(SL_ARRAY_LENGTH(ShaderProperties));
        for (int i = 0; i < SL_ARRAY_LENGTH(ShaderProperties); i++)
        {
            if (ncast<Render::Type>(ShaderProperties[i].API) & API)
            {
                auto &&shader = Create<Shader>(
                    std::string{ AssetsPathes[asset] } + ShaderProperties[i].Path,
                    ShaderProperties[i].Type
                    );
                ShaderContainer.emplace_back(std::move(shader));
            }    
        }
    }

    data.Target = Render::Create<RenderTarget>(RenderTarget::Description{ viewport, { {  Format::RGBA8 }, { Format::Depth } } });
    {
        constexpr float fullScreenVertex[5 * 4] = {
             1.0,  1.0, 0.0, 1.0, 1.0,
            -1.0,  1.0, 0.0, 0.0, 1.0,
            -1.0, -1.0, 0.0, 0.0, 0.0,
             1.0, -1.0, 0.0, 1.0, 0.0
        };

        constexpr UINT32 fullScreenIndices[] = {
            0, 1, 2, 2, 3, 0
        };
        data.FullScreenPipeline = renderer->CreatePipeline(Get<Shader, ShaderName::Texture>());
        data.FullScreenPipeline->Set({
            { Format::VECTOR3, "Position" },
            { Format::VECTOR2, "Texcoord" },
            { Format::VECTOR3, "Normal"   }
        });
        data.FullScreenPipeline->Set(CreateBuffer(sizeof(fullScreenVertex), fullScreenVertex, Buffer::Type::Vertex));
        data.FullScreenPipeline->Set(CreateBuffer(sizeof(fullScreenIndices), fullScreenIndices, Buffer::Type::Index));
        data.FullScreenPipeline->Create(data.Target);
        
        constexpr UINT32 white        = 0xffffffff;
        constexpr UINT32 black        = 0x000000ff;
        constexpr UINT32 transparency = 0x00000000;
        Texture::Description desc = { Format::RGBA8, Texture::Wrap::Repeat, Texture::Filter::Linear };

        data.WhiteTexture       = Render::Create<Texture>(1, 1, &white, desc);
        data.BlackTexture       = Render::Create<Texture>(1, 1, &black, desc);
        data.TransparentTexture = Render::Create<Texture>(1, 1, &transparency, desc);
    }
    Render2D::Setup();
}

void Render::Setup(const std::shared_ptr<RenderTarget> &renderTarget)
{
    Render2D::Setup(renderTarget);
}

void Render::Submit(const std::shared_ptr<Immortal::Shader> &shader, const std::shared_ptr<Mesh> &mesh, const Matrix4 &transform)
{
    shader->Map();
    shader->Set("uTransform", transform);
    shader->Unmap(); 
}

}
