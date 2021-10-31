#include "impch.h"
#include "Render.h"
#include "Render2D.h"

namespace Immortal
{

std::unique_ptr<Renderer> Render::renderer;

std::vector<std::shared_ptr<Shader>> Render::ShaderContainer{};

Render::Scene Render::scene{};

RenderData Render::data{};

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

    auto pipeline = renderer->CreatePipeline(Get<Shader, ShaderName::Texture>());
    pipeline->Set(std::make_shared<Buffer>(), Buffer::Type::Vertex);

    pipeline->Set({
        { Format::VECTOR3, "position" },
        { Format::VECTOR4, "color"    }
        });
        
    int pause = 0;

    {
        /*
        constexpr UINT32 white        = 0xffffffff;
        constexpr UINT32 black        = 0x000000ff;
        constexpr UINT32 transparency = 0x00000000;
        Texture::Description spec = { Texture::Format::RGBA8, Texture::Wrap::Repeat, Texture::Filter::Linear };

        data.WhiteTexture = Texture2D::Create(1, 1, &white, spec);
        data.BlackTexture = Texture2D::Create(1, 1, &black, spec);
        data.TransparentTexture = Texture2D::Create(1, 1, &transparency, spec);

        constexpr float fullScreenVertex[5 * 4] = {
             1.0,  1.0, 0.0, 1.0, 1.0,
            -1.0,  1.0, 0.0, 0.0, 1.0,
            -1.0, -1.0, 0.0, 0.0, 0.0,
             1.0, -1.0, 0.0, 1.0, 0.0
        };

        constexpr UINT32 fullScreenIndices[] = {
            0, 1, 2, 2, 3, 0
        };
        auto fullScreenVertexBuffer = VertexBuffer::Create(fullScreenVertex, sizeof(fullScreenVertex));
        fullScreenVertexBuffer->SetLayout({
            { Shader::DataType::Float3, "position" },
            { Shader::DataType::Float2, "texcoord" }
        });
        auto fullScreenIndexBuffer  = IndexBuffer::Create(fullScreenIndices, sizeof(fullScreenIndices));
        data.FullScreenVertexArray = VertexArray::Create();
        data.FullScreenVertexArray->AddVertexBuffer(fullScreenVertexBuffer);
        data.FullScreenVertexArray->SetIndexBuffer(fullScreenIndexBuffer);*/
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
