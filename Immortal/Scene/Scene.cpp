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
    Matrix4 viewProjectionMatrix;
    Matrix4 skyProjectionMatrix;
    Matrix4 sceneRotationMatrix;
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

    meshes.skybox = std::make_shared<Mesh>("assets/meshes/skybox.obj");
    // textures.skybox.reset(Render::Create<TextureCube>("assets/textures/environment.hdr"));

    uniforms.transform.reset(Render::Create<Buffer>(sizeof(TransformUniformBuffer), 0));
    uniforms.shading.reset(Render::Create<Buffer>(sizeof(ShadingUniformBuffer), 1));

    renderTarget.reset(Render::CreateRenderTarget({
        Resolutions::FHD.Width, Resolutions::FHD.Height,
        {
            { Format::RGBA32F },
            { Format::Depth   }
        }
    }));
         
    pipelines.tonemap = nullptr;
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
    Matrix4 cameraTransform;
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
        renderTarget->Resize(viewportSize);
        Render::Begin(renderTarget);

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

        {
            TransformUniformBuffer transformUniforms;
            transformUniforms.viewProjectionMatrix = primaryCamera->ViewProjection();
            transformUniforms.skyProjectionMatrix  = primaryCamera->Projection() * Matrix4(Vector::Matrix3(primaryCamera->View()));
            transformUniforms.sceneRotationMatrix  = Matrix4(Vector::Matrix3(primaryCamera->View()));
            uniforms.transform->Update(sizeof(TransformUniformBuffer), &transformUniforms);
        }

        {
            ShadingUniformBuffer shadingUniforms;
            shadingUniforms.eyePosition = primaryCamera->View()[3];
            for (int i = 0; i < SL_ARRAY_LENGTH(shadingUniforms.lights); ++i)
            {
                const Light &light = environments.light.lights[i];
                shadingUniforms.lights[i].direction = Vector4{ light.Direction, 0.0f };

                Vector4 finalLight = Vector4{};
                if (light.Enabled)
                {
                    finalLight = Vector4{ light.Radiance, 0.0f };
                }
                shadingUniforms.lights[i].radiance = finalLight;
            }
            uniforms.shading->Update(sizeof(ShadingUniformBuffer), &shadingUniforms);
        }

        {
            auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
            for (auto e : view)
            {
                auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
                auto &shader = Render::Get<Shader, ShaderName::PBR>();
                Render::Submit(shader, mesh.Mesh, transform.Transform());
            }
        }
        Render::End();
    }

}

void Scene::OnRenderEditor(const EditorCamera &editorCamera)
{
    renderTarget->Resize(viewportSize);
    Render::Begin(renderTarget);

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

    {
        TransformUniformBuffer transformUniforms;
        transformUniforms.viewProjectionMatrix = editorCamera.ViewProjection();
        transformUniforms.skyProjectionMatrix  = editorCamera.Projection() * Matrix4(Vector::Matrix3(editorCamera.View()));
        transformUniforms.sceneRotationMatrix  = Matrix4{ Matrix3{ editorCamera.View() } };
        uniforms.shading->Update(sizeof(TransformUniformBuffer), &transformUniforms);
    }

    {
        ShadingUniformBuffer shadingUniforms;
        shadingUniforms.eyePosition = editorCamera.View()[3];
        for (int i = 0; i < SL_ARRAY_LENGTH(shadingUniforms.lights); ++i)
        {
            const Light &light = environments.light.lights[i];
            shadingUniforms.lights[i].direction = Vector4{ light.Direction, 0.0f };

            Vector4 finalLight{};
            if (light.Enabled)
            {
                finalLight = Vector4{ light.Radiance, 0.0f };
            }

            shadingUniforms.lights[i].radiance = finalLight;
        }
        uniforms.shading->Update(sizeof(ShadingUniformBuffer), &shadingUniforms);
    }

    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto e : view)
    {
        auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(e);
        auto &shader = Render::Get<Shader, ShaderName::PBR>();
        Render::Submit(shader, mesh.Mesh, transform.Transform());
    }

    Render::End();
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
