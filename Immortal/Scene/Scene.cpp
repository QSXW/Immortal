#include "Scene.h"

#include "Framework/Timer.h"

#include "Render/Render.h"
#include "Render/Render2D.h"

#include "Object.h"
#include "Component.h"
#include "GameObject.h"
#include "Serializer/SceneSerializer.h"
#include "String/LanguageSettings.h"
#include "Utils/PlatformUtils.h"
#include "ImGui/Utils.h"
#include "Math/Math.h"

#include <cmath>

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

void Scene::RenderAnimatedObject(Ref<Pipeline::Graphics> pipeline, entt::entity object, TransformComponent &transform, MeshComponent &mesh, MaterialComponent &material)
{
    uint32_t baseRegister = 0;
    if (Render::API == Render::Type::Vulkan)
    {
        baseRegister = 2;
    }

    UniformBuffer::Model model;

    auto &animation = mesh.Mesh->GetAnimation()[mesh.Mesh->GetAnimationState()];
    animation.Ticks(Time::DeltaTime);
    mesh.Mesh->CalculatedBoneTransform(transform);

    auto transforms = mesh.Mesh->GetTransforms();

    auto &nodeList = mesh.Mesh->NodeList();
    for (auto &node : nodeList)
    {
        auto &ref = material.References[node.MaterialIndex];

        pipeline->AllocateDescriptorSet(((uint64_t)object << 32) | (uint64_t)&node);
        pipeline->Set(node.Vertex);
        pipeline->Set(node.Index);
        pipeline->Bind(transforms, 7);
        pipeline->Bind(uniforms.transform, 0);
        pipeline->Bind(uniforms.shading, 1);
        pipeline->Bind(ref.Textures.Albedo,    baseRegister + 0);
        pipeline->Bind(ref.Textures.Normal,    baseRegister + 1);
        pipeline->Bind(ref.Textures.Metallic,  baseRegister + 2);
        pipeline->Bind(ref.Textures.Roughness, baseRegister + 3);
        pipeline->Bind(ref.Textures.AO,        baseRegister + 4);

        model.Transform = transform;
        model.Color = ref.AlbedoColor;
        model.Roughness = ref.Roughness;
        model.Metallic = ref.Metallic;
        model.ObjectID = (int)object;

        Render::PushConstant(pipeline, Shader::Stage::Vertex, sizeof(model), &model, 0);
        Render::Draw(pipeline);
    }
}

void Scene::RenderObject(Ref<Pipeline::Graphics> pipeline, entt::entity object, TransformComponent &transform, MeshComponent &mesh, MaterialComponent &material)
{
    uint32_t baseRegister = 0;
    if (Render::API == Render::Type::Vulkan)
    {
        baseRegister = 2;
    }

    UniformBuffer::Model model;
    
    auto &nodeList = mesh.Mesh->NodeList();
    for (auto &node : nodeList)
    {
        auto &ref = material.References[node.MaterialIndex];

        pipeline->AllocateDescriptorSet(((uint64_t)object << 32) | (uint64_t)&node);
        pipeline->Set(node.Vertex);
        pipeline->Set(node.Index);
        pipeline->Bind(uniforms.transform, 0);
        pipeline->Bind(uniforms.shading,   1);
        pipeline->Bind(ref.Textures.Albedo,    baseRegister + 0);
        pipeline->Bind(ref.Textures.Normal,    baseRegister + 1);
        pipeline->Bind(ref.Textures.Metallic,  baseRegister + 2);
        pipeline->Bind(ref.Textures.Roughness, baseRegister + 3);
        pipeline->Bind(ref.Textures.AO,        baseRegister + 4);

        model.Transform = transform;
        model.Color     = ref.AlbedoColor;
        model.Roughness = ref.Roughness;
        model.Metallic  = ref.Metallic;
        model.ObjectID  = (int)object;

        Render::PushConstant(pipeline, Shader::Stage::Vertex, sizeof(model), &model, 0);
        Render::Draw(pipeline);
    }
}

void Scene::ApplyGaussianBlur(Ref<Image> &input, Ref<Image> &output, float sigma, int kernalSize)
{
    struct {
        float blurDirection;
        float kernalSize;
    } properties{
        1,
        static_cast<float>(kernalSize)
    };

    auto width  = input->Width();
    auto height = input->Height();
    Ref<Image> tmp = Render::Create<Image>(width, height, nullptr, Image::Description{Format::RGBA8, Wrap::Repeat, Filter::Bilinear});

    std::vector<float> kernal;
    kernal.resize(properties.kernalSize);
    Math::GenerateGaussianKernal(kernal, sigma);
    uniforms.gaussianKernal->Update(kernal);

    pipelines.horizontalGaussianBlur->AllocateDescriptorSet((uint64_t)&input);
    pipelines.horizontalGaussianBlur->PushConstant(sizeof(properties), &properties);
    pipelines.horizontalGaussianBlur->Bind(uniforms.gaussianKernal, 2);
    pipelines.horizontalGaussianBlur->Bind(input, 0);
    pipelines.horizontalGaussianBlur->Bind(tmp, 1);
    pipelines.horizontalGaussianBlur->Dispatch(SLALIGN(width / 16, 16), SLALIGN(height / 16, 16), 1);

    properties.blurDirection = 0;
    pipelines.verticalGaussianBlur->AllocateDescriptorSet((uint64_t)&input);
    pipelines.verticalGaussianBlur->PushConstant(sizeof(properties), &properties);
    pipelines.verticalGaussianBlur->Bind(uniforms.gaussianKernal, 2);
    pipelines.verticalGaussianBlur->Bind(tmp, 0);
    pipelines.verticalGaussianBlur->Bind(output, 1);
    pipelines.verticalGaussianBlur->Dispatch(SLALIGN(width / 16, 16), SLALIGN(height / 16, 16), 1);
}

void Scene::Init()
{
    auto self = registry.create();
}

void Scene::ReloadSkyBoxCube()
{
    textures.skyboxCube = Render::Create<TextureCube>(settings.environmentResolution, settings.environmentResolution, Texture::Description{ Format::RGBA16F, Wrap::Clamp, Filter::Bilinear, false });
}

void Scene::LoadEnvironment()
{
    meshes.skybox.reset(new Mesh{ "Assets/Meshes/Skybox.obj" });

    float black[] = {
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    textures.skybox = Render::Create<Texture>(2, 1, black, Texture::Description{ Format::RGBA32F, Wrap::Clamp, Filter::Bilinear, false });
    
    ReloadSkyBoxCube();
    Equirect2Cube();
}

void Scene::Equirect2Cube()
{
	if (Render::API == Render::Type::Vulkan)
	{
		pipelines.equirect2Cube->AllocateDescriptorSet((uint64_t) textures.skybox.Get());
		pipelines.equirect2Cube->Bind(textures.skybox, 0);
		pipelines.equirect2Cube->Bind(textures.skyboxCube, 1);
		pipelines.equirect2Cube->Dispatch(SLALIGN(textures.skyboxCube->Width() / 32, 32), SLALIGN(textures.skyboxCube->Height() / 32, 32), 6);
	}
}

Scene::Scene(const std::string &name) :
    name{ name }
{
    Init();
}

Scene::Scene(const std::string &name, bool isEditorScene) :
    name{ name }
{
    Init();

    uniforms.host = Render::Create<Buffer>(sizeof(UniformBuffer::Transform) + sizeof(UniformBuffer::Shading), 0);

    Buffer::BindInfo bindInfo{};
    bindInfo.type   = Buffer::Type::Uniform;

    bindInfo.size   = sizeof(UniformBuffer::Transform);
    bindInfo.offset = 0;
    uniforms.transform = uniforms.host->Bind(bindInfo);

    bindInfo.offset += bindInfo.size;
    bindInfo.size = sizeof(UniformBuffer::Shading);
    uniforms.shading = uniforms.host->Bind(bindInfo);

    renderTarget = Render::CreateRenderTarget({
        Resolutions::FHD.Width, Resolutions::FHD.Height,
        {
            { Format::RGBA8    },
            { Format::R32      },
            { Format::Depth32F }
        }
    });

    constexpr size_t stride = sizeof(SkeletonVertex);
    InputElementDescription inputElementDescription = {
        { Format::VECTOR3, "POSITION" },
        { Format::VECTOR3, "NORMAL"   },
        { Format::VECTOR3, "TANGENT"  },
        { Format::VECTOR2, "TEXCOORD" },
    };
    inputElementDescription.Stride = stride;

    pipelines.animatedBasic = Render::Create<Pipeline::Graphics>(Render::GetShader("AnimatedBasic3D"));
    pipelines.animatedBasic->Set({
        { Format::VECTOR3,  "POSITION"  },
        { Format::VECTOR3,  "NORMAL"    },
        { Format::VECTOR3,  "BITANGENT" },
        { Format::VECTOR2,  "TEXCOORD"  },
        { Format::IVECTOR4, "BONE_ID"   },
        { Format::VECTOR4,  "WEIGHT"    }
        });
    pipelines.animatedBasic->Create(renderTarget);

    pipelines.tonemap = nullptr;
    pipelines.pbr = Render::Create<Pipeline::Graphics>(Render::GetShader(Render::API == Render::Type::Vulkan ? "PhysicalBasedRendering" : "Basic3D"));
    pipelines.pbr->Set(inputElementDescription);
    pipelines.pbr->Create(renderTarget);

    pipelines.basic = Render::Create<Pipeline::Graphics>(Render::GetShader("Basic3D"));
    pipelines.basic->Set(inputElementDescription);
    pipelines.basic->Create(renderTarget);

    InputElementDescription outlineDesc = {
        { Format::VECTOR3, "POSITION" }
    };
    outlineDesc.Stride = stride;

    pipelines.outline = Render::Create<Pipeline::Graphics>(Render::GetShader("Outline"));
    pipelines.outline->Set(outlineDesc);
    pipelines.outline->Disable(Pipeline::State::Depth);
    pipelines.outline->Create(renderTarget);

    InputElementDescription skyboxDesc = {
        { Format::VECTOR3, "POSITION" }
    };
    skyboxDesc.Stride = stride;
    pipelines.skybox = Render::Create<Pipeline::Graphics>(Render::GetShader("Skybox"));
    pipelines.skybox->Set(skyboxDesc);
    pipelines.skybox->Create(renderTarget);

    pipelines.colorMixing = Render::Create<Pipeline::Compute>(Render::GetShader("ColorMixing"));
    pipelines.horizontalGaussianBlur = Render::Create<Pipeline::Compute>(Render::GetShader("GaussianBlur"));
    pipelines.verticalGaussianBlur = Render::Create<Pipeline::Compute>(Render::GetShader("GaussianBlur"));
    pipelines.equirect2Cube = Render::Create<Pipeline::Compute>(Render::GetShader("Equirect2Cube"));

    uniforms.gaussianKernal = Render::CreateBuffer(Limit::MaxGaussianKernalSize * sizeof(float), Buffer::Type{ Buffer::Type::Storage });

    Render2D::Setup(renderTarget);
    LoadEnvironment();
}

Scene::~Scene()
{
    registry.clear();
}

void Scene::OnUpdate()
{

}

void Scene::OnGuiRender()
{
    ImGui::Begin(WordsMap::Get("Scene Editor").c_str());

    ImGui::DragFloat(WordsMap::Get("Exposure").c_str(), &settings.exposure, 0.01f, 0, 50.0f);
    ImGui::DragFloat(WordsMap::Get("Gamma").c_str(), &settings.gamma, 0.01f, 0, 50.0f);
    settings.changed |= ImGui::SliderFloat(WordsMap::Get("Sigma").c_str(), &settings.sigma, 0.1f, 1024.0f);
    settings.changed |= ImGui::SliderInt(WordsMap::Get("KernalSize").c_str(), &settings.kernalSize, 3, Limit::MaxGaussianKernalSize);

    static int item = 5;
    uint32_t resolutions[] = { 64, 128, 256, 512, 1024, 2048, 4096 };

    if (ImGui::Combo(WordsMap::Get("Resolution").c_str(), &item, "64\000128\000256\000512\0001024\0002048\0004096\000"))
    {
        settings.environmentResolution = resolutions[item];
        ReloadSkyBoxCube();
        Equirect2Cube();
    }

    auto [x, y] = ImGui::GetContentRegionAvail();

    ImVec2 size{};
    size.x = x - 8;
    size.y = size.x * textures.skybox->Height() / textures.skybox->Width();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.4509f, 0.7882f, 0.8980f, 1.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 20.0f, 20.0f });
    if (ImGui::ImageButton((ImTextureID)(uint64_t)*textures.skybox, size))
    {
        auto res = FileDialogs::OpenFile(FileFilter::Image);
        if (res.has_value())
        {
            textures.skybox = Render::Create<Texture>(res.value());
            Equirect2Cube();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    ImGui::End();
}

void Scene::OnEvent()
{
    
}

void Scene::OnRenderRuntime()
{
    // Update Script
    {
        registry.view<ScriptComponent>().each([=, this](auto object, ScriptComponent &script) {
                script.Update((int)object, this, Time::DeltaTime);
            });
    }

    SceneCamera *sceneCamera = nullptr;
    {
        auto view = registry.view<TransformComponent, CameraComponent>();
        for (auto &o : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(o);
            if (camera.Primary)
            {
                sceneCamera = &camera.Camera;
                sceneCamera->SetTransform(transform);
                break;
            }
        }
    }

    // Only render when we have a primary Camera
    if (!sceneCamera)
    {
        primaryCamera = dynamic_cast<SceneCamera*>(&observerCamera);
        observerCamera.OnUpdate(Time::DeltaTime);
    }
    else
    {
        primaryCamera = sceneCamera;
    }

    primaryCamera->SetViewportSize(viewportSize);
    OnRender(*primaryCamera);
}

void Scene::OnRenderEditor(const Camera &editorCamera)
{
    OnRender(editorCamera);
}

void Scene::OnRender(const Camera &camera)
{
    float deltaTime = Time::DeltaTime;

    /* Update Video Player Component */
    {
		auto view = registry.view<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>();
        for (auto object : view)
        {
			auto [transform, sprite, videoPlayer, color] = view.get<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>(object);

            auto animator = videoPlayer.GetAnimator();
            animator->Accumulator += deltaTime;
            if (animator->Accumulator < animator->SecondsPerFrame)
            {
                continue;
            }

            animator->Accumulator = fmodf(animator->Accumulator, animator->SecondsPerFrame);
            auto picture = videoPlayer.GetPicture();
            if (picture.Available())
            {
                videoPlayer.PopPicture();
                sprite.UpdateSprite(picture);
                color.Modified = true;
				transform.Scale = Vector3{ sprite.Sprite->Ratio(), 1.0f, 1.0f };
            }
        }
    }

    auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent, ColorMixingComponent>);
    {
        for (auto object : group)
        {
            auto [transform, sprite, color] = group.get<TransformComponent, SpriteRendererComponent, ColorMixingComponent>(object);

            if (!sprite.Sprite)
            {
				continue;
            }

            auto width = sprite.Result->Width();
            auto height = sprite.Result->Height();

            if (color.Modified || !color.Initialized)
            {
                pipelines.colorMixing->AllocateDescriptorSet((uint64_t)object);
                pipelines.colorMixing->PushConstant(ColorMixingComponent::Length, &color.RGBA);
                pipelines.colorMixing->Bind(sprite.Sprite, 0);
                pipelines.colorMixing->Bind(sprite.Result, 1);
                pipelines.colorMixing->Dispatch(SLALIGN(width, 16) >> 4, SLALIGN(height, 16) >> 4, 1);     
                sprite.Result->Blit();
                color.Initialized = true;
            }

            if (settings.changed && Render::API == Render::Type::Vulkan)
            {
                ApplyGaussianBlur(sprite.Sprite, sprite.Result, settings.sigma, settings.kernalSize);
                settings.changed = false;
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
        transform->ViewProjection = camera.ViewProjection();
        transform->SkyboxPorjection = camera.Projection() * Matrix4(Vector::Matrix3(camera.View()));
        transform->SceneRotation = Matrix4{ Matrix3{ camera.View() } };

        auto shading = &buffers.shading;

        size_t i = 0;
        auto lightObjects = registry.view<TransformComponent, LightComponent>();
        for (auto &lightObject : lightObjects)
        {
            if (i >= SL_ARRAY_LENGTH(UniformBuffer::Shading::lights))
            {
                break;
            }
            auto [transform, light] = lightObjects.get<TransformComponent, LightComponent>(lightObject);
            shading->lights[i].direction = Vector4{ transform.Position, 1.0f };

            if (light.Enabled)
            {
                shading->lights[i].radiance = light.Radiance;
            }
            i++;
        }
        shading->CameraPosition = Vector4{ camera.View()[3] };
        shading->Exposure = settings.exposure;
        shading->Gamma    = settings.gamma;

        uniforms.host->Update(sizeof(buffers), &buffers);
    }

    {
        /* Update Skybox */
        if (Render::API == Render::Type::Vulkan)
        {
			auto &nodeList = meshes.skybox->NodeList();
			for (auto &node : nodeList)
			{
				pipelines.skybox->AllocateDescriptorSet((uint64_t) &node);
				pipelines.skybox->Set(node.Vertex);
				pipelines.skybox->Set(node.Index);
				pipelines.skybox->Bind(uniforms.transform, 0);
				pipelines.skybox->Bind(textures.skyboxCube, 1);
				pipelines.skybox->Bind(uniforms.shading, 2);
				Render::Draw(pipelines.skybox);
			}
        }
    }

    {
        if (selectedObject && *selectedObject && selectedObject->HasComponent<MeshComponent>() && Render::API == Render::Type::Vulkan)
        {
            auto &mesh = selectedObject->GetComponent<MeshComponent>();
            auto &transform = selectedObject->GetComponent<TransformComponent>();

            TransformComponent outlineTransform = transform;
            outlineTransform.Scale *= 1.02f;
            struct {
                Matrix4 transform;
                Vector4 color;
                int id;
            } pushConstants{
                outlineTransform,
                Vector4{ 1.0f, 0.0f, 0.0f, 1.0f },
                *selectedObject
            };

            auto &nodeList = mesh.Mesh->NodeList();
            for (auto &node : nodeList)
            {
                pipelines.outline->AllocateDescriptorSet(((uint64_t)selectedObject << 32) | (uint64_t)&node);
                pipelines.outline->Set(node.Vertex);
                pipelines.outline->Set(node.Index);
                pipelines.outline->Bind("Transform", uniforms.transform);
                Render::PushConstant(pipelines.outline, Shader::Stage::Vertex, sizeof(pushConstants), &pushConstants, 0);
                Render::Draw(pipelines.outline);
            }
        }
    }

    auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto object : view)
    {
        auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(object);
        if (Render::API != Render::Type::D3D12 && mesh.Mesh->IsAnimated())
        {
            RenderAnimatedObject(pipelines.animatedBasic, object, transform, mesh, material);
        }
        else
        {
            RenderObject(pipelines.basic, object, transform, mesh, material);
        }
    }

    Render2D::BeginScene(camera);
    for (auto o : group)
    {
        auto [transform, sprite, colorMixing] = group.get<TransformComponent, SpriteRendererComponent, ColorMixingComponent>(o);
        if (sprite.Sprite)
        {
			Render2D::DrawRect(
			    transform,
			    sprite.Result,
			    sprite.TilingFactor, sprite.Color, (int)o);
        }
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

    objects.insert({ name, object });
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

Object Scene::Query(const std::string &name)
{
	auto it = objects.find(name);
	return it != objects.end() ? Object{ it->second, this } : Object{};
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

void Scene::Serialize(const std::string &path)
{
    SceneSerializer{}.Serialize(this, path);
}

bool Scene::Deserialize(const std::string & path)
{
    return SceneSerializer{}.Deserialize(this, path);
}

void Scene::OnKeyPressed(KeyPressedEvent & e)
{
    registry.view<ScriptComponent>().each([=, this](auto object, ScriptComponent &script) {
        script.OnKeyDown((int)object, this, (int)e.GetKeyCode());
        });
}

void Scene::Select(Object *object)
{
    selectedObject = object;
}

}
