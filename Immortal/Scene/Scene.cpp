#include "Scene.h"

#include "Framework/Timer.h"

#include "Render/Graphics.h"
#include "Render/Render2D.h"

#include "Object.h"
#include "Component.h"
#include "GameObject.h"
#include "Serializer/SceneSerializer.h"
#include "String/LanguageSettings.h"
#include "Helper/Platform.h"
#include "ImGui/Utils.h"
#include "Math/Math.h"
#include "Widget/Widget.h"
#include <cmath>

namespace Immortal
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

Scene::Scene(const String &name, bool isEditorScene) :
    name{ name },
    viewportSize{},
    frameGraph{}
{
    Init();
}

Scene::~Scene()
{
    registry.clear();
}

void Scene::Init()
{
	auto self = registry.create();
}

void Scene::OnUpdate()
{

}

void Scene::OnGuiRender()
{
    ImGui::Begin(Translator::Translate("Scene Editor").c_str());

    ImGui::DragFloat(Translator::Translate("Exposure").c_str(), &settings.exposure, 0.01f, 0, 50.0f);
    ImGui::DragFloat(Translator::Translate("Gamma").c_str(), &settings.gamma, 0.01f, 0, 50.0f);
    settings.changed |= ImGui::SliderFloat(Translator::Translate("Sigma").c_str(), &settings.sigma, 0.1f, 1024.0f);
    settings.changed |= ImGui::SliderInt(Translator::Translate("KernalSize").c_str(), &settings.kernalSize, 3, Limit::MaxGaussianKernalSize);

    static int item = 5;
    uint32_t resolutions[] = { 64, 128, 256, 512, 1024, 2048, 4096 };

    if (ImGui::Combo(Translator::Translate("Resolution").c_str(), &item, "64\000128\000256\000512\0001024\0002048\0004096\000"))
    {
        settings.environmentResolution = resolutions[item];
    }

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

    if (!sceneCamera)
    {
        primaryCamera = dynamic_cast<SceneCamera*>(&observerCamera);
    }
    else
    {
        primaryCamera = sceneCamera;
    }

    primaryCamera->SetViewportSize(viewportSize);

    /* Observer needs viewport before OnUpdate (RMB look warps cursor to center of render area). */
    if (!sceneCamera)
    {
        observerCamera.OnUpdate(Time::DeltaTime);
    }
    OnRender(*primaryCamera);
}

void Scene::OnRenderEditor(const Camera &editorCamera)
{
    OnRender(editorCamera);
}

void Scene::OnRender(const Camera &camera)
{
    float deltaTime = Time::DeltaTime;

    if (!render2d)
	{
		render2d = new Render2D;
	}

	CommandBuffer *commandBuffer = Application::Reference().GetCurrentCommandBuffer();

    SceneParameters params{};
	params.view             = camera.View();
    params.viewProjection   = camera.ViewProjection();
	params.invViewProjection = Vector::Inverse(camera.ViewProjection());
	params.cameraWorld       = Vector4{ Vector::Inverse(camera.View())[3] };
	params.skyboxProjection = camera.Projection() * Matrix4(Vector::Matrix3(camera.View()));
	params.exposure         = settings.exposure;
	params.gamma            = settings.gamma;

	{
		size_t li = 0;
		auto lightView = registry.view<TransformComponent, LightComponent>();
		for (auto entity : lightView)
		{
			if (li >= SL_ARRAY_LENGTH(params.lights))
			{
				break;
			}
			auto &transform = lightView.get<TransformComponent>(entity);
			auto &light = lightView.get<LightComponent>(entity);
			if (!light.Enabled)
			{
				continue;
			}
			Matrix3 r3 = Matrix3(Vector::Rotate(transform.Rotation));
			Vector3 fwd = Vector3(r3 * TransformComponent::Forward);
			Vector3 toLight = -Vector::Normalize(fwd);
			// Identity rotation leaves local -Z as (0,0,-1) → toLight = (0,0,1): purely horizontal in Y-up
			// scenes, so N·L is 0 on flat ground and the scene looks unlit. Match the deferred fallback
			// direction used when there are no lights so new / unrotated lights still read as a "sun".
			if (transform.Rotation.Length() < 1e-3f)
			{
				toLight = Vector::Normalize(Vector3{ 0.35f, 0.85f, 0.25f });
			}
			params.lights[li].direction = Vector4{ toLight, 0.0f };
			params.lights[li].radiance = light.Radiance;
			li++;
		}
		params.lightCount = (uint32_t)li;
	}

	frameGraph->Execute(commandBuffer, params);

	if (useDeferredPipeline && gbufferTarget && meshletTask && deferredTask)
	{
		meshletTask->SetRenderPath(MeshletRenderPath::Deferred);
		ClearValue gbufferClear[4] = {
		    { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
		    { .color = { .float32 = { 0.5f, 0.5f, 1.0f, 1.0f } } },
		    { .color = { .uint32 = { 0u, 0u, 0u, 0u } } },
		    { .depthStencil = { .depth = 1.0f, .stencil = 0 } }
		};
		commandBuffer->BeginRenderTarget(gbufferTarget, gbufferClear);
		frameGraph->DrawMesh(commandBuffer, params, registry);
		commandBuffer->EndRenderTarget();
		meshletTask->SetRenderPath(MeshletRenderPath::Forward);
	}

    ClearValue clearValues[3] = {
	    { .color = { 0.0f, 0.0, 0.0, 0.0f }},
		{ .color = { 0 } },
        {.depthStencil = { .depth = 1.0f, .stencil = 0 } }
    };

	commandBuffer->BeginRenderTarget(renderTarget, clearValues);
	frameGraph->Composite(commandBuffer, params);

    /* Update Video Player Component */
    {
        auto view = registry.view<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>();
        for (auto object : view)
        {
            auto [transform, sprite, videoPlayer, color] = view.get<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>(object);
            auto picture = videoPlayer.GetPicture();
            if (picture)
            {
				Graphics::ReleaseResource(sprite.Sprite);
			    Graphics::CreateTexture(picture);
                color.Modified = true;
                transform.Scale = Vector3{ sprite.Sprite->GetRatio(), 1.0f, 1.0f };
            }
        }
    }
        
	if (!useDeferredPipeline)
	{
		frameGraph->DrawMesh(commandBuffer, params, registry);
	}
    Render2DComponent(camera, commandBuffer);

	commandBuffer->EndRenderTarget();
}

void Scene::Render2DComponent(const Camera &camera, CommandBuffer *commandBuffer)
{
	render2d->BeginScene(commandBuffer, camera);
	auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
	{
		for (auto object : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(object);

			if (!sprite.Sprite)
			{
				continue;
			}
			render2d->DrawSprite(transform, sprite, (int) object);
		}
	}
	render2d->EndScene();
}

void Scene::OnRender2D(const Camera &camera, RenderTarget *renderTarget)
{
    if (!render2d)
    {
		render2d = new Render2D;
    }

    CommandBuffer *commandBuffer = Application::Reference().GetCurrentCommandBuffer();
	ClearValue clearValues[3] = {
	    {.color = {0.0f, 0.0, 0.0, 0.0f}},
	    {.color = {0}},
	    {.depthStencil = {.depth = 1.0f, .stencil = 0}}};

    const std::string label = "Render2D";
	commandBuffer->BeginEvent(label.data(), label.size() + 1);
	commandBuffer->BeginRenderTarget(renderTarget, clearValues);
	Render2DComponent(camera, commandBuffer);
	commandBuffer->EndRenderTarget();
	commandBuffer->EndEvent();
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
            //pipelines.pbr->FreeDescriptorSet((uint64_t)&node);
            //pipelines.basic->FreeDescriptorSet((uint64_t)&node);
        }
    }
    else
    {
        //pipelines.pbr->FreeDescriptorSet(object);
    }
    registry.destroy(object);
}

Object Scene::Query(const std::string &name)
{
    auto it = objects.find(name);
    return it != objects.end() ? Object{ it->second, this } : Object{};
}

void Scene::ConfigureDeferredPipeline(bool useDeferred, const Ref<MeshletTask> &meshlet, const Ref<DeferredTask> &deferred)
{
	useDeferredPipeline = useDeferred;
	meshletTask = meshlet;
	deferredTask = deferred;
	if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
	{
		SetViewportSize(viewportSize);
	}
}

void Scene::SetDeferredPBRResolve(bool enable)
{
	if (deferredTask)
	{
		deferredTask->SetUsePBR(enable);
	}
}

bool Scene::IsDeferredPBRResolveEnabled() const
{
	return deferredTask && deferredTask->GetUsePBR();
}

void Scene::SetViewportSize(const Vector2 &size)
{
    viewportSize = size;

    if (renderTarget)
    {
		Graphics::ReleaseResource(renderTarget);
    }

    auto device = Graphics::GetDevice();
	Format colorFormats[] = { Format::RGBA8, Format::R32G32_UINT };
	renderTarget = device->CreateRenderTarget(size.x, size.y, colorFormats, SL_ARRAY_LENGTH(colorFormats), Format::Depth24Stencil8);

	if (gbufferTarget)
	{
		Graphics::ReleaseResource(gbufferTarget);
		gbufferTarget = {};
	}
	if (useDeferredPipeline && deferredTask && meshletTask && size.x > 0.0f && size.y > 0.0f)
	{
		Format gbufferFormats[] = { Format::RGBA8, Format::RGBA8, Format::R32G32_UINT };
		gbufferTarget = device->CreateRenderTarget((uint32_t)size.x, (uint32_t)size.y, gbufferFormats, SL_ARRAY_LENGTH(gbufferFormats), Format::Depth24Stencil8);
		deferredTask->SetGBufferRenderTarget(gbufferTarget);
	}
	else if (deferredTask)
	{
		deferredTask->SetGBufferRenderTarget({});
	}
}

const Vector2 &Scene::GetViewportSize() const
{
	return viewportSize;
}

Ref<Texture> Scene::GetObjectIdPickTexture() const
{
	if (useDeferredPipeline && gbufferTarget)
	{
		return gbufferTarget->GetColorAttachment(2);
	}
	if (renderTarget)
	{
		return renderTarget->GetColorAttachment(1);
	}
	return {};
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


void Scene::SetFrameGraph(const Ref<FrameGraph> &value)
{
	frameGraph = value;
}

}
