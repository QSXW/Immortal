#include "impch.h"
#include "Render.h"
#include "Render2D.h"

namespace Immortal
{

std::unique_ptr<Renderer> Render::renderer;

std::vector<std::shared_ptr<Shader>> Render::ShaderContainer{};

Render::Scene Render::scene{};

Render::Data Render::data{};

static const char *Sringify(Render::Type type)
{
    switch (type)
    {
#define XX(x) case Render::Type::x: return #x;
        XX(None);
        XX(Vulkan);
        XX(D3D12);
#undef XX
        default: return "Unknown";
    }
}

const std::string Render::ShaderProfiles[] = {
    { "Texture"  },
    { "Render2D" }
};

void Render::INIT(RenderContext *context)
{
    LOG::INFO("Initialize Renderer with API => {0}", Sringify(Render::API));
    renderer = Renderer::Create(context);
    renderer->INIT();

    {
        auto asset = API == Type::D3D12 ? 1 : 0;
        ShaderContainer.reserve(SLLEN(ShaderProfiles));
        for (int i = 0; i < SLLEN(ShaderProfiles); i++)
        {
            ShaderContainer.emplace_back(
                    renderer->CreateShader(
                        std::string{ AssetsPathes[asset] } + ShaderProfiles[i],
                        Shader::Type::Graphics
                        )
                );
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
    Render2D::INIT();
}

void Render::Submit(const std::shared_ptr<Immortal::Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const Matrix4 &transform)
{
    shader->Map();
    shader->Set("u_ViewProjection", scene.viewProjectionMatrix);
    shader->Set("u_Transform", transform);

    renderer->DrawIndexed(vertexArray, 1);
}

void Render::Submit(const std::shared_ptr<Immortal::Shader> &shader, const std::shared_ptr<Mesh> &mesh, const Matrix4 &transform)
{
    shader->Map();
    shader->Set("uTransform", transform);
    renderer->DrawIndexed(mesh->VertexArrayObject(), 1);
    shader->Unmap(); 
}

}
