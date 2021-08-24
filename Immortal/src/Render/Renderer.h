#pragma once

#include "ImmortalCore.h"

#include "RenderCommand.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "OrthographicCamera.h"

#ifndef __RENDERER_H__
#define __RENDERER_H__

#define IMMORTAL_RENDER_API_OPENGL
#define IMMORTAL_RENDER_API_D3D12

namespace Immortal {

	struct RendererData
	{
		Ref<Texture2D> BlackTexture;
		Ref<Texture2D> TransparentTexture;
		Ref<Texture2D> WhiteTexture;
		Ref<ShaderMap> mShaderLibrary;
		Ref<VertexArray> FullScreenVertexArray;
	};

	class IMMORTAL_API Renderer
	{
	public:
		enum class ShaderName : INT32
		{
			Texture,
			PBR,
			Skybox,
			Renderer2D,
			Tonemap,
			Test,
			Last
		};

		static std::vector<Ref<Immortal::Shader>> ShaderContainer;

		template <ShaderName INDEX>
		static constexpr inline Ref<Immortal::Shader> Shader()
		{
			static_assert(INDEX < ShaderName::Last, "Shader Index out of bound.");
			return ShaderContainer[static_cast<INT32>(INDEX)];
		}

		static inline Ref<Immortal::Shader> Shader(const ShaderName index)
		{
			size_t i = static_cast<size_t>(index);
			SLASSERT(i < ShaderContainer.size() && "Shader Index out of bound.");
			return ShaderContainer[i];
		}

	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(OrthographicCamera &camera);
		static void EndScene();

		static void __forceinline EnableDepthTest()
		{
			RenderCommand::EnableDepthTest();
		}

		static void __forceinline DisableDepthTest()
		{
			RenderCommand::DisableDepthTest();
		}

		template <class F>
		static void Submit(F Delegate)
		{

		}

		static void WaitForPreviousFrame()
		{

		}

		// Should not be render inmediately. Render before we start rendering
		static void Submit(const Ref<Immortal::Shader> &shader, const Ref<VertexArray> &vertexArray, const Vector::mat4& transform = Vector::mat4(1.0f));

		static void Submit(const Ref<Immortal::Shader> &shader, const Ref<Mesh> &mesh, const Vector::Matrix4 &transform = Vector::Matrix4(1.0f));

		static RendererAPI::Type API()
		{
			return RendererAPI::GetAPI();
		}

		static Ref<ShaderMap> ShaderLibrary() NOEXCEPT
		{
			return Data->mShaderLibrary;
		}

		static Ref<Texture2D> WhiteTexture() NOEXCEPT
		{ 
			return Data->WhiteTexture;
		}

		static Ref<Texture2D> BlackTexture() NOEXCEPT
		{
			return Data->BlackTexture;
		}

		static Ref<Texture2D> TransparentTexture() NOEXCEPT
		{
			return Data->TransparentTexture;
		}

		static Ref<VertexArray> FullScreenVertexArray() NOEXCEPT
		{
			return Data->FullScreenVertexArray;
		}

	private:
		struct SceneData
		{
			Vector::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> sSceneData;

		static const std::string ShaderProfiles[];
		static RendererData *Data;
	};

	using ShaderName = Renderer::ShaderName;

	template <class SuperType, class OPENGL, class VULKAN, class D3D12, class ... Args>
	inline constexpr Ref<SuperType> InstantiateGrphicsPrimitive(Args&& ... args)
	{
		switch (Renderer::API())
		{
		case RendererAPI::Type::OpenGL:
			return CreateRef<OPENGL>(std::forward<Args>(args)...);
		case RendererAPI::Type::VulKan:
			return CreateRef<VULKAN>(std::forward<Args>(args)...);
		case RendererAPI::Type::D3D12:
			return CreateRef<D3D12>(std::forward<Args>(args)...);
		default:
			SLASSERT(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		}
	}

}

#endif /* __RENDERER_H__ */
