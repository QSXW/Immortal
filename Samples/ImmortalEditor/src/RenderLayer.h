#pragma once

#include <Immortal.h>
#include "Panel/Navigator.h"
#include "Panel/HierarchyGraphics.h"
#include "Panel/PropertyManager.h"
#include "Panel/Tools.h"

namespace Immortal
{

class WImGuizmo : public Widget
{
public:
    WImGuizmo(Widget *parent = nullptr) :
        Widget { parent }
    {

    }
};

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewportSize, const std::string &label) :
        Layer{ label },
        eventSink{ this },
        selectedObject{},
        viewportSize{ viewportSize },
        window{ new WWindow },
        viewport{ new WFrame{} },
        editableArea{ new WImage{} },
        imguizmoWidget{ new WImGuizmo{} },
        objectEditorText{new WText },
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

        menus[0] = new WMenu;
        menus[0]
            ->Item({ "Open",       "Ctrl + O", [this] { LoadObject(); }})
            ->Item({ "Save Scene", "Ctrl + S", [this] { SaveScene();  }})
            ->Item({ "Load Scene", "Ctrl + L", [this] { LoadScene();  }})
            ->Item({ "Close",      "Ctrl + W", [this] { Application::App()->Close(); }})
            ->Text("Menu");
        menuBar->Color({0., 0., 0., 1.0f})->AddChild(menus[0]);

        camera.primary = &camera.editor;
        camera.editor = { Vector::PerspectiveFOV(Vector::Radians(90.0f), viewportSize.x, viewportSize.y, 0.1f, 1000.0f) };
        camera.orthographic.SetViewportSize(viewportSize);
        eventSink.Listen(&RenderLayer::OnKeyPressed,    Event::Type::KeyPressed);
        eventSink.Listen(&RenderLayer::OnMouseDown,     Event::Type::MouseButtonPressed);
        eventSink.Listen(&RenderLayer::OnMouseScrolled, Event::Type::MouseScrolled);

        camera.transform.Position = Vector3{ 0.0f, 0.0, -1.0f };

        viewport
            ->Text("Offline Render")
            ->AddChild(
            editableArea
                ->Resize(viewportSize)
                ->Anchors(viewport)
                ->Wrap({
                rightClickMenu
                    ->Padding({ 14.0f, 2.f })
                    ->Width(192.0f)
                    ->Height(260.0f)
                    ->Text("Right Click Menu")
                    ->Color({.15f, .15f, .15f, 1.0f})
                    ->Wrap({ 
                    objectEditorText
                        ->Text("Object Editor")
		                ->Height(10)
                        ->Color({1.0f, 1.0f, 1.0f, .65f}),
                    separator,
                    items.primary
                        ->Color({ 1.0f, 1.0f, 1.0f, 1.0f })
                        ->HoveredColor(ImGui::RGBA32(212, 115, 115, 204))
                        ->Item({ "Select/Deselect", [this] { SelectObject(selectedPosition.x, selectedPosition.y); }})
                        ->Item({ "Import",          [this] { LoadObject(); }})
                        ->Item({ "Load Scene",      [this] { LoadScene();  }})
                        ->Item({ "Save Scene",      [this] { SaveScene();  }}),
                    separator,
                    menus[1]
                        ->Text("Create Object")
                        ->Color({ 1.0f, 1.0f, 1.0f, 1.0f })
                        ->HoveredColor(ImGui::RGBA32(212, 115, 115, 204))
                        ->Item({ "Empty" , "", [this] { Object object = scene->CreateObject("Empty" );                                         }})
                        ->Item({ "Camera", "", [this] { Object object = scene->CreateObject("Camera"); object.AddComponent<CameraComponent>(); }})
                        ->Item({ "Light" , "", [this] { Object object = scene->CreateObject("Light" ); object.AddComponent<LightComponent>();  }}),
                    items.secondary
                        ->Color({ 1.0f, 1.0f, 1.0f, 1.0f })
                        ->HoveredColor(ImGui::RGBA32(212, 115, 115, 204))
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

                imguizmoWidget->Connect([&]() {
                    if (selectedObject && guizmoType != ImGuizmo::OPERATION::INVALID)
                    {
                        auto [x, y] = ImGui::GetWindowPos();
                        auto [w, h] = ImGui::GetWindowSize();

                        const Camera *primaryCamera = nullptr;
                        if (panels.tools->IsControlActive(WTools::Start))
                        {
                            primaryCamera = scene->GetCamera();
                        }
                        else
                        {
                            primaryCamera = camera.primary;
                        }
                        ImGuizmo::SetOrthographic(primaryCamera->IsOrthographic());
                        ImGuizmo::SetDrawlist();
                        ImGuizmo::SetRect(x, y, w, h);

                        TransformComponent &transform = selectedObject.GetComponent<TransformComponent>();
                        Matrix4 munipulatedTransform = transform.Transform();

                        Matrix4 cameraProjectionMatrix = primaryCamera->Projection();
                        Matrix4 cameraViewMatrix = primaryCamera->View();

                        bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
                        float snapValues[][3] = {
                            {  0.5f,  0.5f,  0.5f },
                            { 45.0f, 45.0f, 45.0f },
                            {  0.5f,  0.5f,  0.5f }
                        };

                        ImGuizmo::Manipulate(
                            &cameraViewMatrix[0].x,
                            &cameraProjectionMatrix[0].x,
                            guizmoType,
                            ImGuizmo::LOCAL,
                            &munipulatedTransform[0].x,
                            nullptr,
                            snap ? snapValues[guizmoType] : nullptr);

                        if (ImGuizmo::IsUsing())
                        {
                            Vector3 rotation;
                            Vector::DecomposeTransform(munipulatedTransform, transform.Position, rotation, transform.Scale);

                            Vector3 deltaRotation = rotation - transform.Rotation;
                            transform.Rotation += deltaRotation;
                        }
                    }
                })
                })
            );

    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
        Vector2 size = editableArea->Size();
        if ((size.x != viewportSize.x || size.y != viewportSize.y) &&
            (size.x != 0 && size.y != 0))
        {
            viewportSize = size;
            camera.editor.SetViewportSize(viewportSize);
            camera.orthographic.SetViewportSize(viewportSize);
            scene->SetViewportSize(viewportSize);
        }

        scene->Select(&selectedObject);
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

        if (panels.tools->IsToolActive(WTools::Move))
        {
            guizmoType = ImGuizmo::OPERATION::TRANSLATE;
        }
        else if (panels.tools->IsToolActive(WTools::Rotate))
        {
            guizmoType = ImGuizmo::OPERATION::ROTATE;
        }
        else if (panels.tools->IsToolActive(WTools::Scale))
        {
            guizmoType = ImGuizmo::OPERATION::SCALE;
        }
        else
        {
            guizmoType = ImGuizmo::OPERATION::INVALID;
        }

        Application::App()->GetGuiLayer()->BlockEvent(false);
    }

    void UpdateEditableArea()
    {
        editableArea->Descriptor(*scene->Target());
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
            Ref<Texture> newTexture{ Render::Create<Texture>(res.value()) };
            auto &transform = selectedObject.GetComponent<TransformComponent>();
            transform.Scale = transform.Scale.z * Vector3{ newTexture->Ratio(), 1.0f, 1.0f };

            if (!selectedObject.Has<SpriteRendererComponent>())
            {
                selectedObject.AddComponent<SpriteRendererComponent>();
            }
            auto &sprite = selectedObject.GetComponent<SpriteRendererComponent>();
            sprite.Sprite = newTexture;
            sprite.Result = Render::Create<Texture>(sprite.Sprite->Width(), sprite.Sprite->Height(), nullptr, Texture::Description{ Format::RGBA8, Wrap::Clamp, Filter::Bilinear, false });

            auto &colorMixing = selectedObject.GetComponent<ColorMixingComponent>();
            colorMixing.Initialized = false;
        }
    }

    void SelectObject(float x, float y)
    {
        x -= editableArea->MinBound().x;
        y -= editableArea->MinBound().y;

        uint64_t pixel = scene->Target()->PickPixel(1, x, y, Format::R32);

        Object o = Object{ *(int *)&pixel, scene };
        panels.hierarchyGraphics->Select(pixel == -1 || o == selectedObject ? Object{} : o);
    }

    bool LoadObject()
    {
        Texture::Description desc = {
            Format::RGBA8,
            Wrap::Clamp,
            Filter::Bilinear,
            false
        };
        auto res = FileDialogs::OpenFile(FileFilter::None);
        if (res.has_value())
        {
            const auto &filepath = res.value();
            auto object = scene->CreateObject(res.value());

            panels.hierarchyGraphics->Select(object);

            if (FileSystem::Is3DModel(filepath))
            {
                auto &mesh = object.Add<MeshComponent>();
                mesh.Mesh = std::shared_ptr<Mesh>{ new Mesh{ res.value() } };

                auto &material = object.Add<MaterialComponent>();
                material.References.resize(mesh.Mesh->NodeList().size());
            }
            else if (FileSystem::IsVideo(filepath))
            {
                Ref<Vision::Interface::Demuxer> demuxer = new Vision::FFDemuxer;
                Ref<Vision::VideoCodec> decoder = new Vision::FFCodec;

                Vision::CodedFrame codecFrame;
				demuxer->Open(res.value(), decoder);
				demuxer->Read(&codecFrame);
				decoder->Decode(codecFrame);

                demuxer->Open(filepath, decoder);
                Vision::CodedFrame codedFrame;

                auto &videoPlayer = object.AddComponent<VideoPlayerComponent>(demuxer, decoder);
                auto &sprite = object.AddComponent<SpriteRendererComponent>();
                object.AddComponent<ColorMixingComponent>();
            }
            else if (FileSystem::IsImage(filepath))
            {
                auto &sprite = object.Add<SpriteRendererComponent>();
                sprite.Sprite = Render::Create<Texture>(filepath);
                sprite.Result = Render::Create<Texture>(sprite.Sprite->Width(), sprite.Sprite->Height(), nullptr, desc);

                object.Add<ColorMixingComponent>();
                auto &transform = object.Get<TransformComponent>();
                transform.Scale = Vector3{ sprite.Sprite->Ratio(), 1.0f, 1.0f };
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
            scene->SetViewportSize(viewportSize);
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
                 dcast<Camera*>(&camera.orthographic) :
                 dcast<Camera*>(&camera.editor);
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
                scene->Target()->PickPixel(0, 0, 0, Format::RGBA8);
                Async::Execute([&]() -> void {
                    auto size = editableArea->Size();
                    uint32_t width = U32(size.x);
                    uint32_t height = U32(size.y);

                    uint8_t *dataMapped = nullptr;
                    scene->Target()->Map(0, &dataMapped);

                    Vision::BMPCodec bmp{};
                    bmp.Write("RenderTarget.bmp", width, height, 4, dataMapped, (SLALIGN(width, 8) - width) * 4);

                    scene->Target()->Unmap(0);
                });
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
            guizmoType = ImGuizmo::OPERATION::INVALID;
            break;

        case KeyCode::W:
            guizmoType = ImGuizmo::OPERATION::TRANSLATE;
            panels.tools->Activate(WTools::Move);
            if (control || shift)
            {
                Application::App()->Close();
            }
            break;

        case KeyCode::E:
            panels.tools->Activate(WTools::Rotate);
            guizmoType = ImGuizmo::OPERATION::ROTATE;
            break;

        case KeyCode::R:
            panels.tools->Activate(WTools::Scale);
            guizmoType = ImGuizmo::OPERATION::SCALE;
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
            if (e.GetMouseButton() == MouseCode::Left && Input::IsKeyPressed(KeyCode::Control))
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

    Vector2 viewportSize;

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
    URef<WText> objectEditorText;

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

    Object triangle;

    ImGuizmo::OPERATION guizmoType = ImGuizmo::OPERATION::INVALID;

    Ref<WImGuizmo> imguizmoWidget;
};

}
