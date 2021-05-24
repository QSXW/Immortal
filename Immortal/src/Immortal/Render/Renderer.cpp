#include "impch.h"
#include "Renderer.h"

#include "Renderer2D.h"

namespace Immortal {

	Scope<Renderer::SceneData> Renderer::sSceneData = CreateScope<Renderer::SceneData>();

	std::vector<Ref<Shader>> Renderer::ShaderContainer{};

	const std::string Renderer::ShaderProfiles[] = {
		{ "assets/shaders/Texture.glsl" },
		{ "assets/shaders/PBR.glsl" },
		{ "assets/shaders/skybox.glsl" },
		{ "assets/shaders/Renderer2D.glsl" },
		{ "assets/shaders/Tonemap.glsl"},
		{ "assets/shaders/test.glsl"},
		{ "assets/shaders/mypbr.glsl"}
	};

	RendererData *Renderer::Data = nullptr;

	void Renderer::Init()
	{
		// Static Rendere Data
		{
			Data = new RendererData();
			Data->mShaderLibrary = CreateRef<ShaderMap>();

			for (int i = 0; i < sizeof(ShaderProfiles) / sizeof(ShaderProfiles[0]); i++)
			{
				ShaderContainer.emplace_back(Shader::Create(ShaderProfiles[i]));
			}

			constexpr UINT32 white        = 0xffffffff;
			constexpr UINT32 black        = 0x000000ff;
			constexpr UINT32 transparency = 0x00000000;
			Texture::Description spec = { Texture::Format::RGBA8, Texture::Wrap::Repeat, Texture::Filter::Linear };

			Data->WhiteTexture = Texture2D::Create(1, 1, &white, spec);
			Data->BlackTexture = Texture2D::Create(1, 1, &black, spec);
			Data->TransparentTexture = Texture2D::Create(1, 1, &transparency, spec);

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
			Data->FullScreenVertexArray = VertexArray::Create();
			Data->FullScreenVertexArray->AddVertexBuffer(fullScreenVertexBuffer);
			Data->FullScreenVertexArray->SetIndexBuffer(fullScreenIndexBuffer);
		}
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		delete Data;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera &camera)
	{
		sSceneData->ViewProjectionMatrix = camera.ViewPorjectionMatrix();
	}

	void Renderer::EndScene()
	{
		
	}

	void Renderer::Submit(const Ref<Immortal::Shader>& shader, const Ref<VertexArray>& vertexArray, const Vector::mat4 & transform)
	{
		shader->Bind();
		shader->SetUniform("u_ViewProjection", sSceneData->ViewProjectionMatrix);
		shader->SetUniform("u_Transform", transform);

		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Submit(const Ref<Immortal::Shader> &shader, const Ref<Mesh> &mesh, const Vector::Matrix4 &transform)
	{
		shader->Bind();
		shader->SetUniform("uTransform", transform);
		RenderCommand::DrawIndexed(mesh->VertexArrayObject());
		// Don't forget to unbind shader even though nothing to be done. If not, the GL state change warning would occur.
		shader->Unbind(); 
	}

}