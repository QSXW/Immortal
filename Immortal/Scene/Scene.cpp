#include "impch.h"
#include "Scene.h"

#include "Framework/Application.h"

#include "Render/Render.h"
#include "Render/Render2D.h"

#include "Entity.h"
#include "Component.h"
#include "ScriptableObject.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal
{

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

Scene::Scene(const std::string &debugName, bool isEditorScene) :
    debugName{ debugName }
{
    entity = registry.create();
    registry.emplace<TransformComponent>(entity);
    auto &transform = registry.get<TransformComponent>(entity);

    lightEnvironment.lights[0].Direction = Vector::Normalize(Vector::Vector3{ -1.0f,  0.0f, 0.0f });
    lightEnvironment.lights[1].Direction = Vector::Normalize(Vector::Vector3{ 1.0f,  0.0f, 0.0f });
    lightEnvironment.lights[2].Direction = Vector::Normalize(Vector::Vector3{ 0.0f, -1.0f, 0.0f });

    lightEnvironment.lights[0].Radiance = Vector3{ 1.0f };;
    lightEnvironment.lights[1].Radiance = Vector3{ 1.0f };;
    lightEnvironment.lights[2].Radiance = Vector3{ 1.0f };;

    lightEnvironment.lights[0].Enabled = true;
    lightEnvironment.lights[1].Enabled = true;
    lightEnvironment.lights[2].Enabled = true;


    skybox = std::make_shared<Mesh>("assets/meshes/skybox.obj");
    // skyboxTexture = Render::Create<TextureCube>("assets/textures/environment.hdr");
    environment = std::make_shared<Environment>(skyboxTexture);

    transformUniformBuffer = Render::Create<Buffer>(sizeof(TransformUniformBuffer), 0);
    shadingUniformBuffer   = Render::Create<Buffer>(sizeof(ShadingUniformBuffer), 1);

    renderTarget = Render::CreateRenderTarget({ (uint32_t)1280, (uint32_t)720, { { Format::RGBA32F }, { Format::Depth } } });
         
    toneMap = nullptr;
}

Scene::~Scene()
{
    registry.clear();
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
        registry.view<NativeScriptComponent>().each([=](auto e, NativeScriptComponent &script)
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
        auto view = registry.view<TransformComponent, CameraComponent>();
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
        primaryCamera = dynamic_cast<SceneCamera*>(&observerCamera);
        primaryCamera->SetViewportSize(viewportSize);
        observerCamera.OnUpdate(Application::DeltaTime());
    }
    else
    {
        primaryCamera->SetTransform(cameraTransform);
    }
    {
        renderTarget->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
        Render::Clear(Color{ 0.0f, 0.0f, 0.0f, 1.0 });
#if 0
        // draw skybox
        glDepthMask(GL_FALSE);
        auto &shader = Renderer::Shader<ShaderName::Skybox>();
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shader->Map();
        skyboxTexture->Map();
        Render::DrawIndexed(mSkyBox->VertexArrayObject(), 0);
        glDepthMask(GL_TRUE);
#endif
        {
            Render2D::BeginScene(dynamic_cast<const Camera&>(*primaryCamera));
            auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto e : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(e);
                Render2D::DrawSprite(transform.Transform(), sprite, (int)e);
            }

            Render2D::EndScene();
        }

        // Update transform uniform buffer.
        {
            TransformUniformBuffer transformUniforms;
            transformUniforms.viewProjectionMatrix = primaryCamera->ViewProjection();
            transformUniforms.skyProjectionMatrix = primaryCamera->Projection() * Vector::Matrix4(Vector::Matrix3(primaryCamera->View()));
            transformUniforms.sceneRotationMatrix = Vector::Matrix4(Vector::Matrix3(primaryCamera->View()));
            transformUniformBuffer->Update(sizeof(TransformUniformBuffer), &transformUniforms);
        }

        // Update shading uniform buffer.
        {
            ShadingUniformBuffer shadingUniforms;
            shadingUniforms.eyePosition = primaryCamera->View()[3];
            for (int i = 0; i < 3; ++i)
            {
                const Light& light = lightEnvironment.lights[i];
                shadingUniforms.lights[i].direction = Vector::Vector4{ light.Direction, 0.0f };
                if (light.Enabled)
                {
                    shadingUniforms.lights[i].radiance = Vector::Vector4{ light.Radiance, 0.0f };
                }
                else {
                    shadingUniforms.lights[i].radiance = Vector::Vector4{};
                }
            }
            shadingUniformBuffer->Update(sizeof(ShadingUniformBuffer), &shadingUniforms);
        }

        {
            auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
            for (auto e : view)
            {
                auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
                auto &shader = Render::Get<Shader, ShaderName::PBR>();
                shader->Map();

                shader->Set("uMaterial.AlbedoColor", material.AlbedoColor);
                shader->Set("uMaterial.Metalness", material.Metalness);
                shader->Set("uMaterial.Roughness", material.Roughness);

                material.AlbedoMap->Map(0);
                material.NormalMap->Map(1);
                material.MetalnessMap->Map(2);
                material.RoughnessMap->Map(3);

                skyboxTexture->Map(4);
                // environment->IrradianceMap->Map(5);
                environment->SpecularBRDFLookUpTable->Map(6);
                Render::Submit(shader, mesh.Mesh, transform.Transform());
            }
        }
        renderTarget->Unmap();
    }

}

void Scene::OnRenderEditor(const EditorCamera &editorCamera)
{
    renderTarget->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
    renderTarget->Map();
    Render::Clear(Color{ 0.0f, 0.0f, 0.0f, 1.0 });

    // draw skybox
    Render::DisableDepthTest();
    auto &skyboxShader = Render::Get<Shader, ShaderName::Skybox>();
    skyboxShader->Map();
    skyboxTexture->Map();
    Render::EnableDepthTest();
    skyboxShader->Unmap();

    {
        Render2D::BeginScene(dynamic_cast<const Camera&>(editorCamera));
        auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto e : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(e);
            Render2D::DrawSprite(transform.Transform(), sprite, (int)e);
        }

        Render2D::EndScene();
    }

    // Update transform uniform buffer.
    {
        TransformUniformBuffer transformUniforms;
        transformUniforms.viewProjectionMatrix = editorCamera.ViewProjection();
        transformUniforms.skyProjectionMatrix = editorCamera.Projection() * Vector::Matrix4(Vector::Matrix3(editorCamera.View()));
        transformUniforms.sceneRotationMatrix = Vector::Matrix4(Vector::Matrix3(editorCamera.View()));
        transformUniformBuffer->Update(sizeof(TransformUniformBuffer), &transformUniforms);
    }

    // Update shading uniform buffer.
    {
        ShadingUniformBuffer shadingUniforms;
        shadingUniforms.eyePosition = editorCamera.View()[3];
        for (int i = 0; i < 3; ++i)
        {
            const Light& light = lightEnvironment.lights[i];
            shadingUniforms.lights[i].direction = Vector::Vector4{ light.Direction, 0.0f };
            if (light.Enabled)
            {
                shadingUniforms.lights[i].radiance = Vector::Vector4{ light.Radiance, 0.0f };
            }
            else {
                shadingUniforms.lights[i].radiance = Vector::Vector4{};
            }
        }
        shadingUniformBuffer->Update(sizeof(ShadingUniformBuffer), &shadingUniforms);
    }

    {
        auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
        for (auto e : view)
        {
            auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
            auto &shader = Render::Get<Shader, ShaderName::PBR>();
            shader->Map();

            shader->Set("uMaterial.AlbedoColor", material.AlbedoColor);
            shader->Set("uMaterial.Metalness", material.Metalness);
            shader->Set("uMaterial.Roughness", material.Roughness);

            material.AlbedoMap->Map(0);
            material.NormalMap->Map(1);
            material.MetalnessMap->Map(2);
            material.RoughnessMap->Map(3);
            skyboxTexture->Map(4);
            environment->IrradianceMap->Map(5);
            environment->SpecularBRDFLookUpTable->Map(6);
            Render::Submit(shader, mesh.Mesh, transform.Transform());
        }
    }

    renderTarget->Unmap();
}

Entity Scene::CreateEntity(const std::string & name)
{
    auto e = Entity{ registry.create(), this };

    e.AddComponent<TransformComponent>();
    auto &idComponent = e.AddComponent<IDComponent>();
    entityMap[idComponent.uid] = e;
    e.AddComponent<TagComponent>(name);

    return e;
}

void Scene::DestroyEntity(Entity & e)
{
    registry.destroy(e);
}

void Scene::SetViewportSize(const Vector::Vector2 &size)
{
    viewportSize = size;
}

Entity Scene::PrimaryCameraEntity()
{
    auto view = registry.view<CameraComponent>();
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
