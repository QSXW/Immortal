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
    float Exposure;
    float Gamma;
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

    uniforms.host.reset(Render::Create<Buffer>(sizeof(UniformBuffer::Transform) + sizeof(UniformBuffer::Shading), 0));

    Buffer::BindInfo bindInfo{};
    bindInfo.type   = Buffer::Type::Uniform;

    bindInfo.size   = sizeof(UniformBuffer::Transform);
    bindInfo.offset = 0;
    uniforms.transform.reset(uniforms.host->Bind(bindInfo));

    bindInfo.offset += bindInfo.size;
    bindInfo.size = sizeof(UniformBuffer::Shading);
    uniforms.shading.reset(uniforms.host->Bind(bindInfo));

    renderTarget.reset(Render::CreateRenderTarget({
        Resolutions::FHD.Width, Resolutions::FHD.Height,
        {
            { Format::RGBA8           },
            { Format::R32             },
            { Format::Depth24Stencil8 }
        }
    }));
    // renderTarget->Set(Color{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });
    renderTarget->Set(Colour{ 0.0f, 0.0f, 0.0f, 1 });

    pipelines.tonemap = nullptr;
    pipelines.pbr.reset(Render::Create<Pipeline::Graphics>(Render::Get<Shader, ShaderName::PhysicalBasedRendering>()));
    pipelines.pbr->Set(StandardInputElementDescription);
    pipelines.pbr->Create(renderTarget);
    pipelines.pbr->Bind("Transform", uniforms.transform.get());
    pipelines.pbr->Bind("Shading", uniforms.shading.get());

    pipelines.basic.reset(Render::Create<Pipeline::Graphics>(Render::Get<Shader, ShaderName::Basic3D>()));
    pipelines.basic->CopyState(*pipelines.pbr);
    pipelines.basic->Create(renderTarget);
    pipelines.basic->Bind("Transform", uniforms.transform.get());
    pipelines.basic->Bind("Shading", uniforms.shading.get());

    pipelines.colorMixing.reset(Render::Create<Pipeline::Compute>(Render::Get<Shader, ShaderName::ColorMixing>().get()));
    // pipelines.colorMixing->Bind("Properties", uniforms.properties.get());

    for (size_t i = 0; i < Limit::MaxLightNumber; i++)
    {
        auto &light = CreateObject(std::string{ "Light#" } + std::to_string(i));
        light.AddComponent<LightComponent>();
        light.AddComponent<MaterialComponent>();
        auto &mesh = light.AddComponent<MeshComponent>();
        mesh.Mesh = Mesh::CreateSphere(0.5f);

        auto &transform = light.GetComponent<TransformComponent>();
        transform.Scale = Vector3{ 1.0f, 1.0f, 1.0f };
    }

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
        struct {
            UniformBuffer::Transform transform;
            UniformBuffer::Shading shading;
        } buffers;

        auto transform = &buffers.transform;
        transform->ViewProjection   = camera.ViewProjection();
        transform->SkyboxPorjection = camera.Projection() * Matrix4(Vector::Matrix3(camera.View()));
        transform->SceneRotation    = Matrix4{ Matrix3{ camera.View() } };

        auto shading = &buffers.shading;

        auto &lightObjects = registry.view<TransformComponent, LightComponent>();
        size_t i = 0;
        for (auto &lightObject : lightObjects)
        {
            auto &[transform, light] = lightObjects.get<TransformComponent, LightComponent>(lightObject);
            shading->lights[i].direction = Vector4{ transform.Position, 1.0f };

            if (light.Enabled)
            {
                shading->lights[i].radiance = light.Radiance;
            }
            i++;
        }
        shading->CameraPosition = camera.View()[3];
        shading->Exposure = 4.5f;
        shading->Gamma = 2.2f;

        uniforms.host->Update(sizeof(buffers), &buffers);
    }

    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto object : view)
    {
        auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(object);

        UniformBuffer::Model model;
        model.Transform = transform;
        model.Color     = material.AlbedoColor;
        model.Roughness = material.Roughness;
        model.Metallic  = material.Metallic;
        model.ObjectID  = (int)object;

        auto &nodeList = mesh.Mesh->NodeList();
        for (auto &node : nodeList)
        {
            pipelines.basic->AllocateDescriptorSet((uint64_t)&node);
            pipelines.basic->Set(node.Vertex);
            pipelines.basic->Set(node.Index);
            pipelines.basic->Bind(material.Textures.Albedo.get(), 2);
            pipelines.basic->Bind(material.Textures.Normal.get(), 3);
            pipelines.basic->Bind(material.Textures.Metallic.get(), 4);
            pipelines.basic->Bind(material.Textures.Roughness.get(), 5);
            pipelines.basic->Bind(material.Textures.AO.get(), 6);
            Render::PushConstant(pipelines.basic.get(), Shader::Stage::Vertex, sizeof(model), &model, 0);
            Render::Draw(pipelines.basic);
        }
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
    auto object = Object{ registry.create(), this };
        
    object.AddComponent<TagComponent>(name);
    object.AddComponent<TransformComponent>();
    object.AddComponent<IDComponent>();

    return object;
}

void Scene::DestroyObject(Object &object)
{
    if (!object)
    {
        return;
    }
    if (object.HasComponent<MeshComponent>())
    {
        MeshComponent &mesh = object.GetComponent<MeshComponent>();
        auto &nodeList = mesh.Mesh->NodeList();
        for (auto &node : nodeList)
        {
            pipelines.pbr->FreeDescriptorSet((uint64_t)&node);
            pipelines.basic->FreeDescriptorSet((uint64_t)&node);
        }
    }
    else
    {
        pipelines.pbr->FreeDescriptorSet(object);
    }
    registry.destroy(object);
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
