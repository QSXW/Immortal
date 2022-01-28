#include "impch.h"
#include "Scene.h"

#include "Framework/Application.h"

#include "Render/Render.h"
#include "Render/Render2D.h"

#include "Object.h"
#include "Component.h"
#include "GameObject.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal
{

namespace UniformBuffer
{

struct Transform
{
    Matrix4 ViewProjection;
    Matrix4 SkyboxPorjection;
    Matrix4 SceneRotation;
    Matrix4 Model;
};

struct Shading
{
    struct {
        Vector4 direction;
        Vector4 radiance;
    } lights[4];
    Vector3 CameraPosition;
};

struct Model
{
    Matrix4 Transform;
    Vector3 Color;
    float   Roughness;
    float   Metallic;
    int     ObjectID;
};

}

Scene::Scene(const std::string &debugName, bool isEditorScene) :
    debugName{ debugName }
{
    entity = registry.create();
    registry.emplace<TransformComponent>(entity);

    // meshes.skybox = std::make_shared<Mesh>("assets/meshes/skybox.obj");
    // textures.skybox.reset(Render::Create<TextureCube>("assets/textures/environment.hdr"));

    uniforms.transform.reset(Render::Create<Buffer>(sizeof(UniformBuffer::Transform), 0));
    uniforms.shading.reset(Render::Create<Buffer>(sizeof(UniformBuffer::Shading), 1));
    uniforms.properties.reset(Render::Create<Buffer>(sizeof(ColorMixingComponent) - sizeof(Component::Type), 2));

    renderTarget.reset(Render::CreateRenderTarget({
        Resolutions::FHD.Width, Resolutions::FHD.Height,
        {
            { Format::RGBA8           },
            { Format::R32             },
            { Format::Depth24Stencil8 }
        }
    }));
    renderTarget->Set(Color{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });

    pipelines.tonemap = nullptr;
    pipelines.pbr.reset(Render::Create<Pipeline::Graphics>(Render::Get<Shader, ShaderName::PhysicalBasedRendering>()));
    pipelines.pbr->Set({
        { Format::VECTOR3, "POSITION" },
        { Format::VECTOR3, "NORMAL"   },
        { Format::VECTOR3, "TAGENT"   },
        { Format::VECTOR3, "BITAGENT" },
        { Format::VECTOR2, "TEXCOORD" },
        });
    pipelines.pbr->Create(renderTarget);

    pipelines.pbr->Bind("Transform", uniforms.transform.get());
    pipelines.pbr->Bind("Shading", uniforms.shading.get());

    pipelines.colorMixing.reset(Render::Create<Pipeline::Compute>(Render::Get<Shader, ShaderName::ColorMixing>().get()));
    // pipelines.colorMixing->Bind("Properties", uniforms.properties.get());

    Render2D::Setup(renderTarget);
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
        registry.view<NativeScriptComponent>().each([=](auto o, NativeScriptComponent &script)
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
        for (auto &o : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(o);
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

    OnRender(*primaryCamera);
}

void Scene::OnRenderEditor(const EditorCamera &editorCamera)
{
    OnRender(editorCamera);
}

void Scene::OnRender(const Camera &camera)
{
    auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent, ColorMixingComponent>);
    {
        for (auto o : group)
        {
            auto [transform, sprite, color] = group.get<TransformComponent, SpriteRendererComponent, ColorMixingComponent>(o);

            auto width = sprite.Final->Width();
            auto height = sprite.Final->Height();

            if (color.Modified || !color.Initialized)
            {
                pipelines.colorMixing->ResetResource();
                pipelines.colorMixing->PushConstant(
                    sizeof(ColorMixingComponent) - offsetof(ColorMixingComponent, RGBA),
                    &color.RGBA,
                    0
                );
                pipelines.colorMixing->Bind(sprite.Texture.get(), 0);
                pipelines.colorMixing->Bind(sprite.Final.get(), 1);
                pipelines.colorMixing->Dispatch(width / 16, height / 16, 1);

                color.Initialized = true;
            }
        }
    }

    Render::Begin(renderTarget);

    {
        UniformBuffer::Transform transform;
        transform.ViewProjection   = camera.ViewProjection();
        transform.SkyboxPorjection = camera.Projection() * Matrix4(Vector::Matrix3(camera.View()));
        transform.SceneRotation    = Matrix4{ Matrix3{ camera.View() } };
        uniforms.transform->Update(sizeof(UniformBuffer::Transform), &transform);
    }

    {
        UniformBuffer::Shading shading;
        for (int i = 0; i < SL_ARRAY_LENGTH(shading.lights); ++i)
        {
            const Light &light = environments.light.lights[i];
            shading.lights[i].direction = Vector4{ light.Direction, 0.0f };

            Vector4 finalLight = Vector4{};
            if (light.Enabled)
            {
                finalLight = Vector4{ light.Radiance, 0.0f };
            }
            shading.lights[i].radiance = finalLight;
        }
        shading.CameraPosition = camera.View()[3];
        uniforms.shading->Update(sizeof(UniformBuffer::Shading), &shading);
    }

    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto o : view)
    {
        auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(o);

        UniformBuffer::Model model;
        model.Transform = transform;
        model.Color     = material.AlbedoColor;
        model.Roughness = material.Roughness;
        model.Metallic  = material.Metallic;
        model.ObjectID  = (int)o;

        pipelines.pbr->Set(mesh.Mesh->Get<Buffer::Type::Vertex>());
        pipelines.pbr->Set(mesh.Mesh->Get<Buffer::Type::Index>());
        pipelines.pbr->Bind(material.Textures.Albedo.get(), 2);
        Render::PushConstant(pipelines.pbr.get(), Shader::Stage::Vertex, sizeof(model), &model, 0);
        Render::Draw(pipelines.pbr);
    }

    Render2D::BeginScene(camera);
    for (auto o : group)
    {
        auto [transform, sprite, colorMixing] = group.get<TransformComponent, SpriteRendererComponent, ColorMixingComponent>(o);
        Render2D::DrawQuad(transform, sprite.Final, sprite.TilingFactor, sprite.Color, (int)o);
    }
    Render2D::EndScene();

    Render::End();
}

Object Scene::CreateObject(const std::string &name)
{
    auto o = Object{ registry.create(), this };

    o.AddComponent<TransformComponent>();
    auto &idComponent = o.AddComponent<IDComponent>();
    o.AddComponent<TagComponent>(name);

    return o;
}

void Scene::DestroyObject(Object & o)
{
    registry.destroy(o);
}

void Scene::SetViewportSize(const Vector2 &size)
{
    viewportSize = size;
    renderTarget->Resize(size);
}

Object Scene::PrimaryCameraObject()
{
    auto view = registry.view<CameraComponent>();
    for (auto o : view)
    {
        const auto &camera = view.get<CameraComponent>(o);
        if (camera.Primary)
        {
            return Object{ o, this };
        }
    }
    return Object{};
}

}
