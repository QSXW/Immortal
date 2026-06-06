#pragma once

#include <algorithm>
#include <cmath>

#include <Immortal.h>
#include "Scene/GameScene.h"
#include "Framework/Timer.h"
#include "Panel/HierarchyGraphics.h"
#include "Panel/PropertyManager.h"
#include "Panel/Tools.h"

namespace Immortal
{

enum class SelectionHighlightMode
{
	Outline,
	AABBBox
};

class WImGuizmo : public Widget
{
public:
	using WidgetType = WImGuizmo;
	WIDGET_SET_PROPERTY(Type, type, ImGuizmo::OPERATION, ImGuizmo::OPERATION::INVALID)
	WIDGET_SET_PROPERTY(SelectedObject, selectedObject, Object)
	WIDGET_SET_POINTER(PrimaryCamera, primaryCamera, const Camera)
	WIDGET_SET_PROPERTY(HighLightMode, highlightMode, SelectionHighlightMode, SelectionHighlightMode::Outline);

public:
    WImGuizmo(Widget *parent = nullptr) :
        Widget { parent }
    {

    }

    virtual bool Draw() override
    {
		MeshComponent *meshComp = nullptr;
		if (selectedObject && selectedObject.HasComponent<MeshComponent>())
		{
			meshComp = &selectedObject.GetComponent<MeshComponent>();
		}

		if (selectedObject && highlightMode == SelectionHighlightMode::AABBBox && meshComp && meshComp->Mesh && primaryCamera)
		{
			DrawAABBWireframe(*meshComp);
		}

		if (selectedObject && type != ImGuizmo::OPERATION::INVALID)
		{
			auto [x, y] = ImGui::GetWindowPos();
			auto [w, h] = ImGui::GetWindowSize();

			ImGuizmo::SetOrthographic(primaryCamera->IsOrthographic());
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(x, y, w, h);

			TransformComponent &transform = selectedObject.GetComponent<TransformComponent>();
			const Matrix4 entityMat = transform.Transform();

			bool submeshGizmo = false;
			uint32_t subIdx = 0;
			if (meshComp && meshComp->Mesh && meshComp->Mesh->Size() > 1 && meshComp->SelectedDrawNodeIndex != UINT32_MAX)
			{
				meshComp->EnsureSubmeshLocalCount(meshComp->Mesh->Size());
				submeshGizmo = true;
				subIdx = meshComp->SelectedDrawNodeIndex;
			}

			Vector3 pivot{ 0.0f, 0.0f, 0.0f };
			bool hasPivot = false;
			if (meshComp && meshComp->Mesh)
			{
				if (submeshGizmo)
				{
					pivot = meshComp->Mesh->NodeList()[subIdx].GetAABBCenter();
				}
				else
				{
					pivot = meshComp->Mesh->GetAABBCenter();
				}
				hasPivot = true;
			}

			Matrix4 baseMat = entityMat;
			if (submeshGizmo && meshComp)
			{
				baseMat = entityMat * meshComp->SubmeshLocalTransform[subIdx];
			}

			Matrix4 manipulatedTransform = baseMat;
			if (hasPivot)
			{
				manipulatedTransform = baseMat * Vector::Translate(pivot);
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
				Matrix4 realMat = hasPivot ? manipulatedTransform * Vector::Translate(-pivot) : manipulatedTransform;

				if (submeshGizmo && meshComp)
				{
					meshComp->SubmeshLocalTransform[subIdx] = Vector::Inverse(entityMat) * realMat;
				}
				else
				{
					Vector3 rotation;
					Vector::DecomposeTransform(realMat, transform.Position, rotation, transform.Scale);
					Vector3 deltaRotation = rotation - transform.Rotation;
					transform.Rotation += deltaRotation;
				}
			}
		}

		if (selectedObject && selectedObject.HasComponent<LightComponent>() && selectedObject.HasComponent<TransformComponent>() && primaryCamera)
		{
			DrawLightDirectionInViewport();
		}

        return false;
    }

private:
	Ref<Texture> lightSunIcon;
	bool lightSunIconLoadTried{};

	void EnsureLightSunIcon()
	{
		if (lightSunIconLoadTried)
		{
			return;
		}
		lightSunIconLoadTried = true;
		lightSunIcon = Graphics::CreateTexture("Assets/Icon/sun.png");
	}

	void DrawLightDirectionInViewport()
	{
		const LightComponent &light = selectedObject.GetComponent<LightComponent>();
		const TransformComponent &tc = selectedObject.GetComponent<TransformComponent>();
		if (light.LightType == LightComponent::Type::Point)
		{
			return;
		}

		const Vector3 origin = tc.Position;
		const Vector3 dirToLight = light.DirectionWorld(*primaryCamera, tc);
		Vector3 emitDir = -dirToLight;
		const float elen2 = emitDir.x * emitDir.x + emitDir.y * emitDir.y + emitDir.z * emitDir.z;
		if (elen2 < 1e-12f)
		{
			return;
		}
		emitDir = Vector::Normalize(emitDir);

		const Vector3 camPos = primaryCamera->GetWorldPosition();
		const float distCam = camPos.Distance(origin);
		const float arrowLen = (std::max)(1.05f, (std::min)(16.0f, distCam * 0.088f));

		Vector3 worldUp = TransformComponent::Up;
		if (std::abs(Vector::Dot(emitDir, worldUp)) > 0.92f)
		{
			worldUp = TransformComponent::Right;
		}
		const Vector3 arrowU = Vector::Normalize(Vector::Cross(emitDir, worldUp));
		const Vector3 arrowV = Vector::Normalize(Vector::Cross(arrowU, emitDir));

		const Matrix4 vp = primaryCamera->ViewProjection();
		const auto [wx, wy] = ImGui::GetWindowPos();
		const auto [ww, wh] = ImGui::GetWindowSize();
		ImDrawList *dl = ImGui::GetWindowDrawList();
		const ImU32 colArrow = IM_COL32_WHITE;
		const ImU32 colCube = IM_COL32(255, 200, 95, 220);

		auto project = [&](const Vector3 &p, bool &behind) -> ImVec2 {
			const Vector4 clip = vp * Vector4{ p, 1.0f };
			behind = clip.w <= 1e-4f;
			const float invW = 1.0f / (behind ? 1e-4f : clip.w);
			const float ndcX = clip.x * invW;
			const float ndcY = clip.y * invW;
			return ImVec2(wx + (ndcX * 0.5f + 0.5f) * ww, wy + (-ndcY * 0.5f + 0.5f) * wh);
		};

		bool b0;
		const ImVec2 s0 = project(origin, b0);

		{
			const float cubeHalf = (std::max)(0.28f, (std::min)(5.5f, distCam * 0.055f));
			const Vector3 mn = origin - Vector3{ cubeHalf, cubeHalf, cubeHalf };
			const Vector3 mx = origin + Vector3{ cubeHalf, cubeHalf, cubeHalf };
			const Vector3 corners[8] = {
			    { mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z },
			    { mx.x, mx.y, mn.z }, { mn.x, mx.y, mn.z },
			    { mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z },
			    { mx.x, mx.y, mx.z }, { mn.x, mx.y, mx.z },
			};
			ImVec2 scr[8];
			bool bh[8];
			for (int i = 0; i < 8; ++i)
			{
				scr[i] = project(corners[i], bh[i]);
			}
			static const int edges[12][2] = {
			    {0, 1}, {1, 2}, {2, 3}, {3, 0},
			    {4, 5}, {5, 6}, {6, 7}, {7, 4},
			    {0, 4}, {1, 5}, {2, 6}, {3, 7}
			};
			for (auto &e : edges)
			{
				if (bh[e[0]] || bh[e[1]])
				{
					continue;
				}
				dl->AddLine(scr[e[0]], scr[e[1]], colCube, 1.65f);
			}
		}

		EnsureLightSunIcon();
		if (lightSunIcon && !b0)
		{
			constexpr float kSunBox = 48.0f;
			const ImVec2 half{ kSunBox * 0.5f, kSunBox * 0.5f };
			dl->AddImage(
			    WIMAGE(lightSunIcon),
			    ImVec2(s0.x - half.x, s0.y - half.y),
			    ImVec2(s0.x + half.x, s0.y + half.y),
			    ImVec2(0.0f, 0.0f),
			    ImVec2(1.0f, 1.0f),
			    IM_COL32_WHITE);
		}

		if (!b0)
		{
			const Vector3 a = emitDir;
			const float headL = arrowLen * 0.34f;
			const float shaftL = (std::max)(arrowLen - headL, arrowLen * 0.2f);
			const float shaftHalfW = arrowLen * 0.075f;
			const float headHalfW = arrowLen * 0.2f;
			constexpr float kArrowLine = 1.03f;

			auto drawSeg3 = [&](const Vector3 &p, const Vector3 &q) {
				bool bp, bq;
				const ImVec2 sp = project(p, bp);
				const ImVec2 sq = project(q, bq);
				if (!bp && !bq)
				{
					dl->AddLine(sp, sq, colArrow, kArrowLine);
				}
			};

			auto drawGodotBlockArrowInPlane = [&](const Vector3 &perp) {
				const Vector3 sbP = origin + perp * shaftHalfW;
				const Vector3 sbM = origin - perp * shaftHalfW;
				const Vector3 hb = origin + a * shaftL;
				const Vector3 hbP = hb + perp * shaftHalfW;
				const Vector3 hbM = hb - perp * shaftHalfW;
				const Vector3 hHeadP = hb + perp * headHalfW;
				const Vector3 hHeadM = hb - perp * headHalfW;
				const Vector3 apex = origin + a * arrowLen;

				drawSeg3(sbP, hbP);
				drawSeg3(sbM, hbM);
				drawSeg3(sbP, sbM);
				drawSeg3(hbP, hHeadP);
				drawSeg3(hbM, hHeadM);
				drawSeg3(hHeadP, apex);
				drawSeg3(hHeadM, apex);
				drawSeg3(hHeadP, hHeadM);
			};

			drawGodotBlockArrowInPlane(arrowU);
			drawGodotBlockArrowInPlane(arrowV);
		}
	}

	void DrawAABBWireframe(MeshComponent &mc)
	{
		TransformComponent &tc = selectedObject.GetComponent<TransformComponent>();
		Matrix4 world = tc.Transform();

		Vector3 mn, mx;
		if (mc.SelectedDrawNodeIndex != UINT32_MAX && mc.SelectedDrawNodeIndex < mc.Mesh->Size())
		{
			auto &node = mc.Mesh->NodeList()[mc.SelectedDrawNodeIndex];
			mn = node.AABBMin;
			mx = node.AABBMax;
			if (mc.SubmeshLocalTransform.size() > mc.SelectedDrawNodeIndex)
			{
				world = world * mc.SubmeshLocalTransform[mc.SelectedDrawNodeIndex];
			}
		}
		else
		{
			mc.Mesh->GetAABB(mn, mx);
		}

		Vector3 corners[8] = {
			{ mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z },
			{ mx.x, mx.y, mn.z }, { mn.x, mx.y, mn.z },
			{ mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z },
			{ mx.x, mx.y, mx.z }, { mn.x, mx.y, mx.z },
		};

		Matrix4 vp = primaryCamera->ViewProjection();
		auto [wx, wy] = ImGui::GetWindowPos();
		auto [ww, wh] = ImGui::GetWindowSize();

		ImVec2 screenPts[8];
		bool behind[8];
		for (int i = 0; i < 8; i++)
		{
			Vector4 clip = vp * (world * Vector4{ corners[i], 1.0f });
			behind[i] = clip.w <= 0.0001f;
			float invW = 1.0f / (behind[i] ? 0.0001f : clip.w);
			float ndcX = clip.x * invW;
			float ndcY = clip.y * invW;
			screenPts[i].x = wx + (ndcX * 0.5f + 0.5f) * ww;
			screenPts[i].y = wy + (-ndcY * 0.5f + 0.5f) * wh;
		}

		static const int edges[12][2] = {
			{0,1},{1,2},{2,3},{3,0},
			{4,5},{5,6},{6,7},{7,4},
			{0,4},{1,5},{2,6},{3,7}
		};

		ImDrawList *dl = ImGui::GetWindowDrawList();
		ImU32 col = IM_COL32(255, 165, 10, 255);
		for (auto &e : edges)
		{
			if (behind[e[0]] || behind[e[1]]) continue;
			dl->AddLine(screenPts[e[0]], screenPts[e[1]], col, 1.5f);
		}
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
        panels.propertyManager = new WPropertyManager;
        panels.hierarchyGraphics = new WHierarchyGraphics([this](Object object) { selectedObject = object; });
        panels.hierarchyGraphics->SetLoadSceneHandler([this]() { LoadScene(); });
        window->Wrap({ menuBar, viewport, panels.tools, panels.propertyManager, panels.hierarchyGraphics});

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

        eventSink.Listen(&RenderLayer::OnKeyPressed,    Event::Type::KeyPressed);
        eventSink.Listen(&RenderLayer::OnMouseDown,     Event::Type::MouseButtonPressed);
        eventSink.Listen(&RenderLayer::OnMouseScrolled, Event::Type::MouseScrolled);

		menus[0]->Item({ "Selection: AABB / Outline", "", [this] {
			if (imguizmoWidget->HighLightMode() == SelectionHighlightMode::Outline)
			{
				imguizmoWidget->HighLightMode(SelectionHighlightMode::AABBBox);
			}
			else
			{
				imguizmoWidget->HighLightMode(SelectionHighlightMode::Outline);
			}
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
            }

            scene->Select(&selectedObject);
            if (auto *atmos = scene->GetAtmosphereTask())
            {
                atmosphereDayTime += Time::DeltaTime;
                float a = atmosphereDayTime * 0.052f;
                float el = 0.38f + 0.52f * sinf(a);
                el += 0.035f * sinf(atmosphereDayTime * 0.28f);
                el = fmaxf(-0.22f, fminf(0.92f, el));
                float xz = sqrtf(fmaxf(0.f, 1.f - el * el));
                Vector3 sunDir{ xz * cosf(a), el, xz * sinf(a) };
                atmos->SetSunDirection(sunDir.Normalize());
                atmos->SetDayPhase(atmosphereDayTime);
            }
            if (panels.tools->IsControlActive(WTools::Start))
            {
                scene->OnRenderRuntime();
            }
            else
            {
                if (viewport->IsHovered())
                {
                    if (Camera *host = scene->GetEditorPrimaryCamera())
                    {
                        host->OnUpdate();
                    }
                }
                scene->OnRenderEditor();
            }
        }

        UpdateEditableArea();
        UpdateRightClickMenu();

        panels
            .hierarchyGraphics
            ->OnUpdate(scene);

        panels
            .propertyManager
            ->OnUpdate(selectedObject);

        panels
            .tools
            ->OnUpdate(selectedObject);

        imguizmoWidget->SelectedObject(selectedObject);
        imguizmoWidget->PrimaryCamera(
            panels.tools->IsControlActive(WTools::Start) ? scene->GetCamera() : scene->GetEditorPrimaryCamera());

		if (auto *ot = scene->GetOutlineTask())
		{
			ot->SetEnabled(imguizmoWidget->HighLightMode() == SelectionHighlightMode::Outline);
		}

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
        asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
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

            std::filesystem::path path = res.value().GetWString();
			auto object = scene->CreateObject((char *)path.stem().u8string().c_str());

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
            scene.Reset(new GameScene{ FileSystem::ExtractFileName(path.value()), true });
            scene->Deserialize(path.value());
			scene->SetViewportSize(editableArea->GetSize());
            panels.hierarchyGraphics->OnUpdate(scene);
            selectedObject = {};
            panels.hierarchyGraphics->Select({});
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
            scene->SetCameraType(
                scene->GetCameraType() == SceneCameraType::EditorPerspective
                    ? SceneCameraType::EditorOrthographic
                    : SceneCameraType::EditorPerspective);
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
				SaveScene();
            }
            break;

        case KeyCode::F:
            if (!!selectedObject && scene->GetCameraType() == SceneCameraType::EditorPerspective)
            {
                scene->GetEditorCamera().Focus(selectedObject.GetComponent<TransformComponent>().Position);
            }
            break;

        case KeyCode::Q:
            panels.tools->Activate(WTools::Hand);
			imguizmoWidget->Type(ImGuizmo::OPERATION::INVALID);
            break;

        case KeyCode::W:
			imguizmoWidget->Type(ImGuizmo::OPERATION::TRANSLATE);
            panels.tools->Activate(WTools::Move);
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
        return true;
    }

    virtual void OnEvent(Event &e) override
    {
        eventSink.Dispatch(e);
		if (viewport->IsHovered())
		{
			if (Camera *host = scene->GetEditorPrimaryCamera())
			{
				host->OnEvent(e);
			}
		}
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

    Ref<GameScene> scene{ new GameScene{ "RenderLayer", true }};

    float atmosphereDayTime = 0.0f;

    Object triangle;

    Ref<WImGuizmo> imguizmoWidget;

    Ref<Mesh> meshLoading;

    URef<AsyncComputeThread> asyncComputeThread;

    Ref<Buffer> selectedBuffer;
};

}
