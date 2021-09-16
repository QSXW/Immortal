#include "impch.h"
#include "Render.h"
#include "Renderer2D.h"

namespace Immortal
{

std::unique_ptr<Renderer> Render::handle;

std::vector<Ref<Shader>> Render::ShaderContainer{};

Render::Scene Render::scene{};

RenderData Render::data{};

const std::string Render::ShaderProfiles[] = {
    { "assets/shaders/texture" },
    { "assets/shaders/PBR" },
    { "assets/shaders/skybox" },
    { "assets/shaders/Renderer2D" },
    { "assets/shaders/Tonemap"},
    { "assets/shaders/test"},
    { "assets/shaders/mypbr"}
};

void Render::INIT(RenderContext *context)
{
    handle = Renderer::Create(context);
    handle->INIT();

    {
        data.ShaderLibrary = CreateRef<ShaderMap>();

        for (int i = 0; i < sizeof(ShaderProfiles) / sizeof(ShaderProfiles[0]); i++)
        {
            ShaderContainer.emplace_back(context->CreateShader(ShaderProfiles[i]));
        }

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
        data.FullScreenVertexArray->SetIndexBuffer(fullScreenIndexBuffer);
    }
    Renderer2D::INIT();
}

void Render::Submit(const Ref<Immortal::Shader> &shader, const Ref<VertexArray> &vertexArray, const Matrix4 &transform)
{
    shader->Map();
    shader->Set("u_ViewProjection", scene.viewProjectionMatrix);
    shader->Set("u_Transform", transform);

    handle->DrawIndexed(vertexArray, 1);
}

void Render::Submit(const Ref<Immortal::Shader> &shader, const Ref<Mesh> &mesh, const Matrix4 &transform)
{
    shader->Map();
    shader->Set("uTransform", transform);
    handle->DrawIndexed(mesh->VertexArrayObject(), 1);
    shader->Unmap(); 
}

}
