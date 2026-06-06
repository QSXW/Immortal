#include "Scene.h"

#include "Framework/Timer.h"

#include "Render/Graphics.h"
#include "Shared/Log.h"
#include "Render/Render2D.h"

#include "Object.h"
#include "Component.h"
#include "GameObject.h"
#include "SceneSerializer.h"
#include "String/LanguageSettings.h"
#include <string>

namespace Immortal
{

Scene::Scene(const String &name, bool isEditorScene_) :
    name{ name },
    viewportSize{},
    isEditorScene{ isEditorScene_ }
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

}

void Scene::OnEvent()
{

}

void Scene::OnRenderRuntime()
{
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

    if (!sceneCamera)
    {
        observerCamera.OnUpdate(Time::DeltaTime);
    }

    if (!render2d)
    {
        render2d = new Render2D;
    }

    CommandBuffer *commandBuffer = Application::Reference().GetCurrentCommandBuffer();
    commandBuffer->BeginRenderTarget(renderTarget, renderTargetClearValues.data());

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
                transform.Scale = Vector3{ picture.GetDisplayAspectRatio(), 1.0f, 1.0f };
            }
        }
    }

    Render2DComponent(*primaryCamera, commandBuffer);
    commandBuffer->EndRenderTarget();
}

void Scene::Render2DComponent(const Camera &camera, CommandBuffer *commandBuffer)
{
	render2d->BeginScene(commandBuffer, camera);
	auto view = registry.view<TransformComponent, SpriteRendererComponent>();
	for (auto object : view)
	{
		auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(object);

		if (!sprite.Sprite)
		{
			continue;
		}
		render2d->DrawSprite(transform, sprite, (int)object);
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
    const std::string label = "Render2D";
	commandBuffer->BeginEvent(label);
	commandBuffer->BeginRenderTarget(renderTarget, renderTargetClearValues.data());
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
        }
    }
    else
    {
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

    Format formats{Format::R8G8B8A8_UNORM};
    ClearValue clears[] = {
        { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
        { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
    };
	Graphics::ReleaseResource(renderTarget);
	renderTarget = Graphics::GetDevice()->CreateRenderTarget(size.x, size.y, &formats, 1, Format::Depth24Stencil8, clears);
}

const Vector2 &Scene::GetViewportSize() const
{
	return viewportSize;
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

bool Scene::Deserialize(const std::string &path)
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
