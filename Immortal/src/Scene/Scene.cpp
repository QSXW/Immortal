#include "impch.h"
#include "Scene.h"

#include "Framework/Application.h"

#include "Render/Renderer.h"
#include "Render/Renderer2D.h"

#include "Entity.h"
#include "Component.h"
#include "ScriptableObject.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal {

	struct TransformUniformBuffer
	{
		Vector::mat4 viewProjectionMatrix;
		Vector::mat4 skyProjectionMatrix;
		Vector::mat4 sceneRotationMatrix;
	};

	struct ShadingUniformBuffer
	{
		struct {
			Vector4 direction;
			Vector4 radiance;
		} lights[3];
		Vector4 eyePosition;
	};

	Scene::Scene(const std::string & debugName, bool isEditorScene)
	{
		 mEntity = mRegistry.create();
		 mRegistry.emplace<TransformComponent>(mEntity);
		 auto &transform = mRegistry.get<TransformComponent>(mEntity);

		 mLightEnvironment.lights[0].Direction = Vector::Normalize(Vector::Vector3{ -1.0f,  0.0f, 0.0f });
		 mLightEnvironment.lights[1].Direction = Vector::Normalize(Vector::Vector3{ 1.0f,  0.0f, 0.0f });
		 mLightEnvironment.lights[2].Direction = Vector::Normalize(Vector::Vector3{ 0.0f, -1.0f, 0.0f });

		 mLightEnvironment.lights[0].Radiance = Vector::Vector3{ 1.0f };;
		 mLightEnvironment.lights[1].Radiance = Vector::Vector3{ 1.0f };;
		 mLightEnvironment.lights[2].Radiance = Vector::Vector3{ 1.0f };;

		 mLightEnvironment.lights[0].Enabled = true;
		 mLightEnvironment.lights[1].Enabled = true;
		 mLightEnvironment.lights[2].Enabled = true;


		 mSkyBox = CreateRef<Mesh>("assets/meshes/skybox.obj");
		 mSkyboxTexture = TextureCube::Create("assets/textures/environment.hdr");
		 mEnvironment = CreateRef<Environment>(mSkyboxTexture);

		 mTransformUniformBuffer = UniformBuffer::Create(sizeof(TransformUniformBuffer), 0);
		 mShadingUniformBuffer   = UniformBuffer::Create(sizeof(ShadingUniformBuffer), 1);

		 mFramebuffer = Framebuffer::Create({ (uint32_t)1280, (uint32_t)720, { { Texture::Format::RGBA32F }, { Texture::Format::Depth } } });
		 
		 mToneMap = Renderer::FullScreenVertexArray();
	}

	Scene::~Scene()
	{
		mRegistry.clear();
	}

	void Scene::OnUpdate()
	{
		
	}

	void Scene::OnEvent()
	{

	}

	void Scene::OnRenderRuntime()
	{
		// Update Script
		{
			mRegistry.view<NativeScriptComponent>().each([=](auto e, NativeScriptComponent &script)
				{
					if (script.Status == NativeScriptComponent::Status::Ready)
					{
						script.OnRuntime();
					}
				});
		}

		SceneCamera *primaryCamera = nullptr;
		Vector::mat4 cameraTransform;
		{
			auto view = mRegistry.view<TransformComponent, CameraComponent>();
			for (auto &e : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
				if (camera.Primary)
				{
					primaryCamera = &camera.Camera;
					cameraTransform = transform;
					break;
				}
			}
		}

		// Only renderer when we have a primary Camera
		if (!primaryCamera)
		{
			primaryCamera = dynamic_cast<SceneCamera*>(&mObserverCamera);
			primaryCamera->SetViewportSize(mViewportSize.x, mViewportSize.y);
			mObserverCamera.OnUpdate(Application::DeltaTime());
		}
		else
		{
			primaryCamera->SetTransform(cameraTransform);
		}
		{
			mFramebuffer->Resize(static_cast<uint32_t>(mViewportSize.x), static_cast<uint32_t>(mViewportSize.y));
			mFramebuffer->Map();
			RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0 });
			RenderCommand::Clear();
#if 0
			// draw skybox
			glDepthMask(GL_FALSE);
			auto &shader = Renderer::Shader<ShaderName::Skybox>();
			// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shader->Map();
			mSkyboxTexture->Map();
			RenderCommand::DrawIndexed(mSkyBox->VertexArrayObject(), 0);
			glDepthMask(GL_TRUE);
#endif
			{
				Renderer2D::BeginScene(dynamic_cast<const Camera&>(*primaryCamera));
				auto group = mRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto e : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(e);
					Renderer2D::DrawSprite(transform.Transform(), sprite, (int)e);
				}

				Renderer2D::EndScene();
			}

			// Update transform uniform buffer.
			{
				TransformUniformBuffer transformUniforms;
				transformUniforms.viewProjectionMatrix = primaryCamera->ViewProjection();
				transformUniforms.skyProjectionMatrix = primaryCamera->Projection() * Vector::Matrix4(Vector::Matrix3(primaryCamera->View()));
				transformUniforms.sceneRotationMatrix = Vector::Matrix4(Vector::Matrix3(primaryCamera->View()));
				mTransformUniformBuffer->SetData(sizeof(TransformUniformBuffer), &transformUniforms);
			}

			// Update shading uniform buffer.
			{
				ShadingUniformBuffer shadingUniforms;
				shadingUniforms.eyePosition = primaryCamera->View()[3];
				for (int i = 0; i < 3; ++i)
				{
					const Light& light = mLightEnvironment.lights[i];
					shadingUniforms.lights[i].direction = Vector::Vector4{ light.Direction, 0.0f };
					if (light.Enabled)
					{
						shadingUniforms.lights[i].radiance = Vector::Vector4{ light.Radiance, 0.0f };
					}
					else {
						shadingUniforms.lights[i].radiance = Vector::Vector4{};
					}
				}
				mShadingUniformBuffer->SetData(sizeof(ShadingUniformBuffer), &shadingUniforms);
			}

			{
				auto view = mRegistry.view<TransformComponent, MeshComponent, MaterialComponent>();
				for (auto e : view)
				{
					auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
					auto &shader = Renderer::Shader<ShaderName::PBR>();
					shader->Map();

					shader->SetUniform("uMaterial.AlbedoColor", material.AlbedoColor);
					shader->SetUniform("uMaterial.Metalness", material.Metalness);
					shader->SetUniform("uMaterial.Roughness", material.Roughness);

					material.AlbedoMap->Map(0);
					material.NormalMap->Map(1);
					material.MetalnessMap->Map(2);
					material.RoughnessMap->Map(3);

					mSkyboxTexture->Map(4);
					mEnvironment->IrradianceMap->Map(5);
					mEnvironment->SpecularBRDFLookUpTable->Map(6);
					Renderer::Submit(shader, mesh.Mesh, transform.Transform());
				}
			}
			mFramebuffer->Unmap();
		}

	}

	void Scene::OnRenderEditor(const EditorCamera &editorCamera)
	{
		mFramebuffer->Resize(static_cast<uint32_t>(mViewportSize.x), static_cast<uint32_t>(mViewportSize.y));
		mFramebuffer->Map();
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0 });
		RenderCommand::Clear();

		// draw skybox
		RenderCommand::DisableDepthTest();
		auto &skyboxShader = Renderer::Shader<ShaderName::Skybox>();
		skyboxShader->Map();
		mSkyboxTexture->Map();
		RenderCommand::DrawIndexed(mSkyBox->VertexArrayObject(), 0);
		RenderCommand::EnableDepthTest();
		skyboxShader->Unmap();

		{
			Renderer2D::BeginScene(dynamic_cast<const Camera&>(editorCamera));
			auto group = mRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto e : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(e);
				Renderer2D::DrawSprite(transform.Transform(), sprite, (int)e);
			}

			Renderer2D::EndScene();
		}

		// Update transform uniform buffer.
		{
			TransformUniformBuffer transformUniforms;
			transformUniforms.viewProjectionMatrix = editorCamera.ViewProjection();
			transformUniforms.skyProjectionMatrix = editorCamera.Projection() * Vector::Matrix4(Vector::Matrix3(editorCamera.View()));
			transformUniforms.sceneRotationMatrix = Vector::Matrix4(Vector::Matrix3(editorCamera.View()));
			mTransformUniformBuffer->SetData(sizeof(TransformUniformBuffer), &transformUniforms);
		}

		// Update shading uniform buffer.
		{
			ShadingUniformBuffer shadingUniforms;
			shadingUniforms.eyePosition = editorCamera.View()[3];
			for (int i = 0; i < 3; ++i)
			{
				const Light& light = mLightEnvironment.lights[i];
				shadingUniforms.lights[i].direction = Vector::Vector4{ light.Direction, 0.0f };
				if (light.Enabled)
				{
					shadingUniforms.lights[i].radiance = Vector::Vector4{ light.Radiance, 0.0f };
				}
				else {
					shadingUniforms.lights[i].radiance = Vector::Vector4{};
				}
			}
			mShadingUniformBuffer->SetData(sizeof(ShadingUniformBuffer), &shadingUniforms);
		}

		{
			auto view = mRegistry.view<TransformComponent, MeshComponent, MaterialComponent>();
			for (auto e : view)
			{
				auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
				auto &shader = Renderer::Shader<ShaderName::PBR>();
				shader->Map();

				shader->SetUniform("uMaterial.AlbedoColor", material.AlbedoColor);
				shader->SetUniform("uMaterial.Metalness", material.Metalness);
				shader->SetUniform("uMaterial.Roughness", material.Roughness);

				material.AlbedoMap->Map(0);
				material.NormalMap->Map(1);
				material.MetalnessMap->Map(2);
				material.RoughnessMap->Map(3);
				mSkyboxTexture->Map(4);
				mEnvironment->IrradianceMap->Map(5);
				mEnvironment->SpecularBRDFLookUpTable->Map(6);
				Renderer::Submit(shader, mesh.Mesh, transform.Transform());
			}
		}

		mFramebuffer->Unmap();
	}

	Entity Scene::CreateEntity(const std::string & name)
	{
		auto e = Entity{ mRegistry.create(), this };

		e.AddComponent<TransformComponent>();
		auto &idComponent = e.AddComponent<IDComponent>();
		mEntityMap[idComponent.uid] = e;
		e.AddComponent<TagComponent>(name);

		return e;
	}

	void Scene::DestroyEntity(Entity & e)
	{
		mRegistry.destroy(e);
	}

	void Scene::SetViewportSize(const Vector::Vector2 &size)
	{
		mViewportSize = size;
	}

	Entity Scene::PrimaryCameraEntity()
	{
		auto view = mRegistry.view<CameraComponent>();
		for (auto e : view)
		{
			const auto &camera = view.get<CameraComponent>(e);
			if (camera.Primary)
			{
				return Entity{ e, this };
			}
		}
		return Entity{};
	}
}
