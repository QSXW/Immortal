#pragma once

#include <Immortal.h>
#include "Framework/Timer.h"
#include "Render/AtmosphereTask.h"
#include "Render/DeferredTask.h"
#include "Render/IBLTask.h"
#include "Render/MeshletTask.h"
#include "Render/SkyboxTask.h"
#include "Panel/Navigator.h"
#include "Panel/HierarchyGraphics.h"
#include "Panel/PropertyManager.h"
#include "Panel/Tools.h"

namespace Immortal
{

class WImGuizmo : public Widget
{
public:
	using WidgetType = WImGuizmo;
	WIDGET_SET_PROPERTY(Type, type, ImGuizmo::OPERATION, ImGuizmo::OPERATION::INVALID)
	WIDGET_SET_PROPERTY(SelectedObject, selectedObject, Object)
	WIDGET_SET_POINTER(PrimaryCamera, primaryCamera, const Camera)

public:
    WImGuizmo(Widget *parent = nullptr) :
        Widget { parent }
    {

    }

    virtual bool Draw() override
    {
		if (selectedObject && type != ImGuizmo::OPERATION::INVALID)
		{
			auto [x, y] = ImGui::GetWindowPos();
			auto [w, h] = ImGui::GetWindowSize();

			ImGuizmo::SetOrthographic(primaryCamera->IsOrthographic());
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(x, y, w, h);

			TransformComponent &transform = selectedObject.GetComponent<TransformComponent>();
			const Matrix4 parent = transform.Transform();

			bool submeshGizmo = false;
			MeshComponent *meshComp = nullptr;
			uint32_t subIdx = 0;
			if (selectedObject.HasComponent<MeshComponent>())
			{
				meshComp = &selectedObject.GetComponent<MeshComponent>();
				if (meshComp->Mesh && meshComp->Mesh->Size() > 1 && meshComp->SelectedDrawNodeIndex != UINT32_MAX)
				{
					meshComp->EnsureSubmeshLocalCount(meshComp->Mesh->Size());
					submeshGizmo = true;
					subIdx = meshComp->SelectedDrawNodeIndex;
				}
			}

			Matrix4 manipulatedTransform = parent;
			if (submeshGizmo && meshComp)
			{
				manipulatedTransform = parent * meshComp->SubmeshLocalTransform[subIdx];
			}

			Matrix4 cameraProjectionMatrix = primaryCamera->Projection();
			Matrix4 cameraViewMatrix = primaryCamera->View();

			bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
			float snapValues[][3] = {
			    {0.5f, 0.5f, 0.5f},
			    {45.0f, 45.0f, 45.0f},
			    {0.5f, 0.5f, 0.5f}};

			ImGuizmo::Manipulate(
			    &cameraViewMatrix[0].x,
			    &cameraProjectionMatrix[0].x,
			    type,
			    ImGuizmo::LOCAL,
			    &manipulatedTransform[0].x,
			    nullptr,
			    snap ? snapValues[type] : nullptr);

			if (ImGuizmo::IsUsing())
			{
				if (submeshGizmo && meshComp)
				{
					meshComp->SubmeshLocalTransform[subIdx] = Vector::Inverse(parent) * manipulatedTransform;
				}
				else
				{
					Vector3 rotation;
					Vector::DecomposeTransform(manipulatedTransform, transform.Position, rotation, transform.Scale);

					Vector3 deltaRotation = rotation - transform.Rotation;
					transform.Rotation += deltaRotation;
				}
			}
		}

        return false;
    }
};

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewportSize, const std::string &label) :
        Layer{ label },
        eventSink{ this },
        selectedObject{},
        window{ new WWindow },
        viewport{ new WFrame{} },
        editableArea{ new WImage{} },
        imguizmoWidget{ new WImGuizmo{} },
	    objectEditorText{new WTextRectangle},
        separator{ new WSeparator },
        menuBar{ new WMenuBar },
        rightClickMenu{new WPopup}
    {
        menus[1] = new WMenu;
        items.primary = new WItemList;
        items.secondary = new WItemList;

        panels.tools = new WTools;
        panels.navigator = new WNavigator([this] { OnTextureLoaded(); });
        panels.propertyManager = new WPropertyManager;
        panels.hierarchyGraphics = new WHierarchyGraphics([this](Object object) { selectedObject = object; });
        window->Wrap({ menuBar, viewport, panels.tools, panels.navigator, panels.propertyManager, panels.hierarchyGraphics});

        asyncComputeThread = new AsyncComputeThread(Graphics::GetDevice());
		asyncComputeThread->Execute<SetQueueTask>(Graphics::GetDevice()->CreateQueue(QueueType::Compute));

        menus[0] = new WMenu;
        menus[0]
            ->Item({ "Open",       "Ctrl + O", [this] { LoadObject(); }})
            ->Item({ "Save Scene", "Ctrl + S", [this] { SaveScene();  }})
            ->Item({ "Load Scene", "Ctrl + L", [this] { LoadScene();  }})
            ->Item({ "Close",      "Ctrl + W", [this] { Application::This->Close(); }})
            ->Text("Menu");
        menuBar->Color(0xff000000)->AddChild(menus[0]);

        camera.primary = &camera.editor;
        camera.editor = { Vector::PerspectiveFOV(Vector::Radians(90.0f), viewportSize.x, viewportSize.y, 0.1f, 1000.0f) };
        camera.orthographic.SetViewportSize(viewportSize);
        eventSink.Listen(&RenderLayer::OnKeyPressed,    Event::Type::KeyPressed);
        eventSink.Listen(&RenderLayer::OnMouseDown,     Event::Type::MouseButtonPressed);
        eventSink.Listen(&RenderLayer::OnMouseScrolled, Event::Type::MouseScrolled);

        camera.transform.Position = Vector3{ 0.0f, 0.0, -1.0f };

        Ref<FrameGraph> frameGraph = new FrameGraph{};

        static constexpr bool kUseAtmosphereSky = false;
        iblTask = new IBLTask;
        if constexpr (kUseAtmosphereSky)
        {
            atmosphereSky = new AtmosphereTask;
            atmosphereSky->SetQuality(AtmosphereQuality::High);
            atmosphereSky->SetSunIntensity(20);
            frameGraph->AddTask(atmosphereSky);
        }
        else
        {
            skyboxTask = new SkyboxTask;
            skyboxTask->SetFilePath("skybox.hdr");
            frameGraph->AddTask(skyboxTask);
        }

        frameGraph->AddTask(iblTask, kUseAtmosphereSky ? "Atmosphere" : "Skybox");

        meshletTask = new MeshletTask;
		frameGraph->AddTask(meshletTask);

		// Deferred: G-buffer pass in MeshletTask + fullscreen lighting in DeferredLighting (depends on Meshlet in graph order).
		static constexpr bool kUseDeferredMeshlet = true;
		if constexpr (kUseDeferredMeshlet)
		{
			deferredTask = new DeferredTask;
			frameGraph->AddTask(deferredTask, "Meshlet");
		}

		frameGraph->Build();
		scene->SetFrameGraph(frameGraph);
		if (skyboxTask)
		{
			iblTask->LinkSkybox(skyboxTask.Get());
		}
		if (deferredTask)
		{
			deferredTask->SetIBLTask(iblTask.Get());
		}
		scene->ConfigureDeferredPipeline(kUseDeferredMeshlet, meshletTask, deferredTask);
		scene->SetDeferredPBRResolve(deferredPBRResolve);
		menus[0]->Item({ "Toggle Deferred PBR", "", [this] {
			deferredPBRResolve = !deferredPBRResolve;
			scene->SetDeferredPBRResolve(deferredPBRResolve);
		}});

        uint64_t nullPick = 0;
		selectedBuffer = Graphics::CreateBuffer(BufferType::TransferDestination, sizeof(uint64_t), &nullPick);

		GuiLayer::AddFont("Assets/Fonts/NotoSansCJKsc-Regular.otf", 18, nullptr);

        viewport
            ->Text("Offline Render")
            ->AddChild(
            editableArea
                ->Resize(viewportSize)
                ->Anchors(viewport)
		        ->PaddingTop(0)
		        ->PaddingBottom(0)
		        ->PaddingLeft(0)
		        ->PaddingRight(0)
                ->Wrap({
                rightClickMenu
                    ->Padding({ 14.0f, 2.f })
                    ->Width(192.0f)
                    ->Height(260.0f)
                    ->Text("Right Click Menu")
                    ->Color(0xff262626)
                    ->Wrap({
                  //  objectEditorText
                  //      ->Text("Object Editor")
		                //->Height(10)
                  //      ->Color(0xa5ffffff),
                    separator,
                    items.primary
                        ->Color(0xffffffff)
                        ->HoveredColor(0xcc7373d4)
                        ->Item({ "Select/Deselect", [this] { SelectObject(selectedPosition.x, selectedPosition.y); }})
                        ->Item({ "Import",          [this] { LoadObject(); }})
                        ->Item({ "Load Scene",      [this] { LoadScene();  }})
                        ->Item({ "Save Scene",      [this] { SaveScene();  }}),
                    separator,
                    menus[1]
                        ->Text("Create Object")
                        ->Color(0xffffffff)
		                                    ->HoveredColor(0xcc7373d4)
                        ->Item({ "Empty" , "", [this] { Object object = scene->CreateObject("Empty" );                                         }})
                        ->Item({ "Camera", "", [this] { Object object = scene->CreateObject("Camera"); object.AddComponent<CameraComponent>(); }})
                        ->Item({ "Light" , "", [this] { Object object = scene->CreateObject("Light" ); object.AddComponent<LightComponent>();  }}),
                    items.secondary
                        ->Color(0xffffffff)
                        ->HoveredColor(0xcc7373d4)
                        ->Item({ "Copy",            [this] { CopyObject(); }})
                        ->Item({ "Paste",           [this] {               }})
                        ->Item({ "Duplicate",       [this] { panels.hierarchyGraphics->Select(CopyObject()); }})
                        ->Item({ "Delete",          [this] {
                            if (selectedObject)
                            {
                                panels.navigator
                                    ->Select(Object{});
                                panels.propertyManager
                                    ->Select(Object{});
                                panels.hierarchyGraphics
                                    ->Select(Object{});
                                scene->DestroyObject(selectedObject);
                                selectedObject = Object{};
                            }
                        }}),
                    separator
                    }),
		           imguizmoWidget
                })
            );

    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
		Vector2 size = editableArea->GetSize();

        if (size.x > 0 && size.y > 0)
		{
		    auto &viewportSize = scene->GetViewportSize();
            if ((size.x != viewportSize.x || size.y != viewportSize.y) &&
                (size.x != 0 && size.y != 0))
            {
				scene->SetViewportSize(size);
				camera.editor.SetViewportSize(size);
				camera.orthographic.SetViewportSize(size);
            }

            scene->Select(&selectedObject);
            if (atmosphereSky)
            {
                atmosphereDayTime += Time::DeltaTime;
                // Full day cycle (sun can dip); slow azimuth + slight wobble for visible motion.
                float a = atmosphereDayTime * 0.052f;
                float el = 0.38f + 0.52f * sinf(a);
                el += 0.035f * sinf(atmosphereDayTime * 0.28f);
                el = fmaxf(-0.22f, fminf(0.92f, el));
                float xz = sqrtf(fmaxf(0.f, 1.f - el * el));
                Vector3 sunDir{ xz * cosf(a), el, xz * sinf(a) };
                atmosphereSky->SetSunDirection(sunDir.Normalize());
                atmosphereSky->SetDayPhase(atmosphereDayTime);
            }
            if (panels.tools->IsControlActive(WTools::Start))
            {
                scene->OnRenderRuntime();
            }
            else
            {
                if (viewport->IsHovered())
                {
                    camera.primary->OnUpdate();
                }
                scene->OnRenderEditor(*camera.primary);
            }
        }

        UpdateEditableArea();
        UpdateRightClickMenu();

        panels
            .hierarchyGraphics
            ->OnUpdate(scene);

        panels
            .navigator
            ->OnUpdate(selectedObject);

        panels
            .propertyManager
            ->OnUpdate(selectedObject);

        panels
            .tools
            ->OnUpdate(selectedObject);

        imguizmoWidget->SelectedObject(selectedObject);
        imguizmoWidget->PrimaryCamera(panels.tools->IsControlActive(WTools::Start) ? scene->GetCamera() : camera.primary);

        if (panels.tools->IsToolActive(WTools::Move))
        {
			imguizmoWidget->Type(ImGuizmo::OPERATION::TRANSLATE);
        }
        else if (panels.tools->IsToolActive(WTools::Rotate))
        {
			imguizmoWidget->Type(ImGuizmo::OPERATION::ROTATE);
        }
        else if (panels.tools->IsToolActive(WTools::Scale))
        {
			imguizmoWidget->Type(ImGuizmo::OPERATION::SCALE);
        }
        else
        {
			imguizmoWidget->Type(ImGuizmo::OPERATION::INVALID);
        }

        //Application::This->Getgui()->BlockEvent(false);
    }

    void UpdateEditableArea()
    {
		auto renderTarget = scene->GetRenderTarget();
        if (renderTarget)
        {
			editableArea->Source(renderTarget->GetColorAttachment(0));
        }
    }

    void UpdateRightClickMenu()
    {
        if (viewport->IsHovered() && !ImGuizmo::IsOver() &&
            Input::IsMouseButtonPressed(MouseCode::Right) && !Input::IsKeyPressed(KeyCode::Control) && !Input::IsKeyPressed(KeyCode::Alt)
            && !panels.tools->IsControlActive(WTools::Start))
        {
            ImVec2 pos = ImGui::GetMousePos();
            selectedPosition = *(Vector2 *)&pos;
            rightClickMenu->Trigger(true);
        }
        else
        {
            rightClickMenu->Trigger(false);
        }
    }

    void OnTextureLoaded()
    {
        auto res = FileDialogs::OpenFile(FileFilter::Image);
        if (res.has_value())
        {
            Ref<Texture> newTexture{ Graphics::CreateTexture(res.value()) };
            auto &transform = selectedObject.GetComponent<TransformComponent>();
            transform.Scale = transform.Scale.z * Vector3{ newTexture->GetRatio(), 1.0f, 1.0f };

            if (!selectedObject.Has<SpriteRendererComponent>())
            {
                selectedObject.AddComponent<SpriteRendererComponent>();
            }
            auto &sprite = selectedObject.GetComponent<SpriteRendererComponent>();
            sprite.Sprite = newTexture;
            sprite.Result = Graphics::CreateTexture(Format::RGBA8, sprite.Sprite->GetWidth(), sprite.Sprite->GetHeight());

            auto &colorMixing = selectedObject.GetComponent<ColorMixingComponent>();
            colorMixing.Initialized = false;
        }
    }

    void SelectObject(float x, float y)
    {
        x -= editableArea->MinBound().x;
        y -= editableArea->MinBound().y;

        auto asyncComputeThread = Graphics::GetAsyncComputeThread();
        asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::BeginRecording);
        asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t value, CommandBuffer *commandBuffer) {
			auto texture = scene->GetObjectIdPickTexture();
			if (!texture)
			{
				return;
			}
            Rect2D rect = {
                .left   = (uint32_t)x,
                .top    = (uint32_t)y,
                .right  = (uint32_t)x + 1,
                .bottom = (uint32_t)y + 1
            };
			commandBuffer->CopyImageToBuffer(selectedBuffer, texture, 0, SLALIGN(texture->GetWidth() * texture->GetFormat().GetTexelSize(), TextureAlignment), &rect);
		});
		asyncComputeThread->Execute<ExecutionCompletedTask>([this] {
			uint32_t *map = nullptr;
			selectedBuffer->Map((void **)&map, sizeof(uint64_t), 0);
            if (map)
            {
				const uint32_t entityRaw = map[0];
				const uint32_t subPick = map[1];
				if (entityRaw == 0)
				{
					panels.hierarchyGraphics->Select(Object{});
					return;
				}
				Object o = Object{ (int)entityRaw, scene };
				if (!o)
				{
					panels.hierarchyGraphics->Select(Object{});
					return;
				}
				bool samePick = (o == selectedObject);
				if (samePick && o.HasComponent<MeshComponent>() && o.GetComponent<MeshComponent>().Mesh)
				{
					auto &mc = o.GetComponent<MeshComponent>();
					const size_t n = mc.Mesh->NodeList().size();
					if (n > 1)
					{
						samePick = (mc.SelectedDrawNodeIndex == subPick);
					}
				}
				if (samePick)
				{
					panels.hierarchyGraphics->Select(Object{});
					return;
				}
				panels.hierarchyGraphics->Select(o);
				if (o.HasComponent<MeshComponent>())
				{
					auto &mc = o.GetComponent<MeshComponent>();
					const size_t nodeCount = mc.Mesh ? mc.Mesh->NodeList().size() : 0;
					if (nodeCount > 1)
					{
						mc.SelectedDrawNodeIndex = subPick < nodeCount ? subPick : (uint32_t)(nodeCount - 1);
					}
					else
					{
						mc.SelectedDrawNodeIndex = UINT32_MAX;
					}
				}
            }
            });
		asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::EndRecording);
		asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::Submiting);
    }

    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile();
        if (res.has_value())
        {
            const auto &filepath = res.value();
            auto object = scene->CreateObject(res.value());

            panels.hierarchyGraphics->Select(object);

            if (FileSystem::Is3DModel(filepath) ||  FileSystem::IsFormat<FileFormat::BIN>(filepath))
            {
                auto meshComponent = &object.Add<MeshComponent>();
				auto material      = &object.Add<MaterialComponent>();

				asyncComputeThread->Begin();
		        meshLoading = new Mesh{asyncComputeThread, nullptr, res.value()};

				asyncComputeThread->Execute<ExecutionCompletedTask>([=, this] {
					meshComponent->Mesh = meshLoading;
					meshComponent->Mesh->PopulateMaterialComponent(*material);
				});

				asyncComputeThread->End();
				asyncComputeThread->Submit();
            }
            else if (FileSystem::IsVideo(filepath))
            {
                auto &videoPlayer = object.AddComponent<VideoPlayerComponent>(res.value());
                auto &sprite = object.AddComponent<SpriteRendererComponent>();
                object.AddComponent<ColorMixingComponent>();
            }
            else if (FileSystem::IsImage(filepath))
            {
				Graphics::GetAsyncComputeThread()->Execute<AsyncTask>(AsyncTaskType::BeginRecording);
                auto &sprite = object.Add<SpriteRendererComponent>();
                sprite.Sprite = Graphics::CreateTexture(filepath);
                sprite.Result = Graphics::CreateTexture(Format::RGBA8, sprite.Sprite->GetWidth(), sprite.Sprite->GetHeight());
				Graphics::GetAsyncComputeThread()->Execute<AsyncTask>(AsyncTaskType::EndRecording);
				Graphics::GetAsyncComputeThread()->Execute<AsyncTask>(AsyncTaskType::Submiting);

                object.Add<ColorMixingComponent>();
                auto &transform = object.Get<TransformComponent>();
                transform.Scale = Vector3{ sprite.Sprite->GetRatio(), 1.0f, 1.0f };
            }

            return true;
        }

        return false;
    }

    void SaveScene()
    {
        auto path = FileDialogs::SaveFile(FileFilter::Scene);
        if (path.has_value())
        {
            scene->Serialize(path.value());
        }
    }

    void LoadScene()
    {
        auto path = FileDialogs::OpenFile(FileFilter::Scene);
        if (path.has_value())
        {
            scene.Reset(new Scene{ FileSystem::ExtractFileName(path.value()), true });
            scene->SetViewportSize(editableArea->GetSize());
            scene->Deserialize(path.value());
            panels.hierarchyGraphics->OnUpdate(scene);
        }
    }

    Object CopyObject()
    {
        if (selectedObject)
        {
            Object dst = scene->CreateObject();

            selectedObject.CopyTo(dst);
            return dst;
        }
        return Object{};
    }

    bool OnKeyPressed(KeyPressedEvent &e)
    {
        if (panels.tools->IsControlActive(WTools::Start))
        {
            scene->OnKeyPressed(e);
            return false;
        }

        if (e.RepeatCount() > 0)
        {
            return false;
        }

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool shift   = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
        switch (e.GetKeyCode())
        {
        case KeyCode::C:
            camera.primary = (camera.primary == &camera.editor) ?
                 (Camera *)(&camera.orthographic) :
                 (Camera *)(&camera.editor);
            break;

        case KeyCode::L:
            if (control)
            {
                if (Input::IsKeyPressed(KeyCode::LeftAlt))
                {
                    LoadScene();
                }
                else
                {
                    LoadObject();
                }
            }
            break;

        case KeyCode::S:
            if (control)
            {
                //scene->Target()->PickPixel(0, 0, 0, Format::RGBA8);
                //Async::Execute([&]() -> void {
                //    auto size = editableArea->Size();
                //    uint32_t width = U32(size.x);
                //    uint32_t height = U32(size.y);

                //    uint8_t *dataMapped = nullptr;
                //    scene->Target()->Map(0, &dataMapped);

                //    Vision::BMPCodec bmp{};
                //    bmp.Write("RenderTarget.bmp", width, height, 4, dataMapped, (SLALIGN(width, 8) - width) * 4);

                //    scene->Target()->Unmap(0);
                //});
            }
            break;

        case KeyCode::F:
            if (!!selectedObject && camera.primary == &camera.editor)
            {
                camera.editor.Focus(selectedObject.GetComponent<TransformComponent>().Position);
            }
            break;

        case KeyCode::Q:
            panels.tools->Activate(WTools::Hand);
			imguizmoWidget->Type(ImGuizmo::OPERATION::INVALID);
            break;

        case KeyCode::W:
			imguizmoWidget->Type(ImGuizmo::OPERATION::TRANSLATE);
            panels.tools->Activate(WTools::Move);
            //if (control || shift)
            //{
            //    Application::This->Close();
            //}
            break;

        case KeyCode::E:
            panels.tools->Activate(WTools::Rotate);
			imguizmoWidget->Type(ImGuizmo::OPERATION::ROTATE);
            break;

        case KeyCode::R:
            panels.tools->Activate(WTools::Scale);
			imguizmoWidget->Type(ImGuizmo::OPERATION::SCALE);
            break;

        case KeyCode::N:
        default:
            return false;
        }

        return true;
    }

    bool OnMouseDown(MouseButtonPressedEvent &e)
    {
		if (viewport->IsHovered() && !ImGuizmo::IsOver())
        {
			/* Ctrl+LMB: pick entity + submesh (RT pick = uint2). Skip Alt+LMB (camera orbit). */
            if (e.GetMouseButton() == MouseCode::Left && Input::IsKeyPressed(KeyCode::Control) && !Input::IsKeyPressed(KeyCode::LeftAlt))
            {
                auto [x, y] = ImGui::GetMousePos();
                SelectObject(x, y);
                return true;
            }
        }

        return false;
    }

    bool OnMouseScrolled(MouseScrolledEvent &e)
    {
		if (viewport->IsHovered())
        {
            return camera.primary->OnMouseScrolled(e);
        }
        return true;
    }

    virtual void OnEvent(Event &e) override
    {
        eventSink.Dispatch(e);
    }

private:
    URef<WWindow> window;

    Ref<RenderTarget> renderTarget;

    Ref<Shader> shader;

    Object selectedObject;

    struct {
        Ref<GraphicsPipeline> graphics;
    } pipelines;

    struct {
        Camera *primary;

        EditorCamera editor;

        OrthographicCamera orthographic;

        TransformComponent transform;
    } camera;

    struct {
        URef<WNavigator> navigator;
        URef<WHierarchyGraphics> hierarchyGraphics;
        URef<WPropertyManager> propertyManager;
        URef<WTools> tools;
    } panels;

    struct {
        bool showDemoWindow{ true };
    } Settings;

    Ref<WFrame> viewport;

    Ref<WImage> editableArea;

    URef<WPopup> rightClickMenu;

    struct
    {
        URef<WItemList> primary;
        URef<WItemList> secondary;
    } items;

    URef<WSeparator> separator;
    URef<WTextRectangle> objectEditorText;

    URef<WMenuBar> menuBar;
    std::array<URef<WMenu>, 6> menus;

    Vector2 selectedPosition;

    struct
    {
        Matrix4 viewProjection;
        Matrix4 modeTransform;
    } ubo;

    std::shared_ptr<Buffer> uniformBuffer;

    Object cameraObject;

    EventSink<RenderLayer> eventSink;

    Ref<Scene> scene{ new Scene{ "RenderLayer", true }};

    Ref<AtmosphereTask> atmosphereSky;

    Ref<SkyboxTask> skyboxTask;

    Ref<IBLTask> iblTask;

    Ref<MeshletTask> meshletTask;

    Ref<DeferredTask> deferredTask;

    float atmosphereDayTime = 0.0f;

    Object triangle;

    Ref<WImGuizmo> imguizmoWidget;

    Ref<Mesh> meshLoading;

    URef<AsyncComputeThread> asyncComputeThread;

    Ref<Buffer> selectedBuffer;

	/** When deferred is enabled, use Cook-Torrance resolve instead of simple N·L (Menu → Toggle Deferred PBR). */
	bool deferredPBRResolve = true;
};

}
