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
        viewport{ new WFrame{} },
        editableArea{ new WImage{} },
        imguizmoWidget{ new WImGuizmo{} }
    {
        camera.primary = &camera.editor;
        camera.editor = { Vector::PerspectiveFOV(Vector::Radians(90.0f), viewportSize.x, viewportSize.y, 0.1f, 1000.0f) };
        camera.orthographic.SetViewportSize(viewportSize);
        eventSink.Listen(&RenderLayer::OnKeyPressed,    Event::Type::KeyPressed);
        eventSink.Listen(&RenderLayer::OnMouseDown,     Event::Type::MouseButtonPressed);
        eventSink.Listen(&RenderLayer::OnMouseScrolled, Event::Type::MouseScrolled);

        camera.transform.Position = Vector3{ 0.0f, 0.0, -1.0f };

        viewport->AddChild(editableArea)->AddChild(imguizmoWidget);
        viewport->Text(WordsMap::Get("Offline Render"));
        editableArea->Resize(viewportSize).Anchors(viewport);

        imguizmoWidget->Connect([&]() {
            if (selectedObject && guizmoType != ImGuizmo::OPERATION::INVALID)
            {
                auto [x, y] = ImGui::GetWindowPos();
                auto [w, h] = ImGui::GetWindowSize();

                const Camera *primaryCamera = nullptr;
                if (panels.tools.IsControlActive(Tools::Start))
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
            });
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
        if (panels.tools.IsControlActive(Tools::Start))
        {
            scene->OnRenderRuntime();
        }
        else
        {
            if (editableArea->IsHovered())
            {
                camera.primary->OnUpdate();
            }
            scene->OnRenderEditor(*camera.primary);
        }
    }

    void UpdateEditableArea()
    {
        editableArea->Descriptor(*scene->Target());
        viewport->Render();
    }

    void UpdateMenuBar()
    {
        bool isSaveAsClicked = false;
        auto *gui = Application::App()->GetGuiLayer();
        gui->UpdateTheme();

        ImGui::PushFont(gui->Bold());
        menuBar.OnUpdate([&]() -> void {
            if (ImGui::BeginMenu(WordsMap::Get("Menu")))
            {
                if (ImGui::MenuItem(WordsMap::Get("Open"), "Ctrl + O"))
                {
                    LoadObject();
                }
                if (ImGui::MenuItem(WordsMap::Get("Load Object"), "Ctrl + L"))
                {
                    LoadObject();
                }
                if (ImGui::MenuItem(WordsMap::Get("Save"), "Ctrl + S"))
                {

                }
                if (ImGui::MenuItem(WordsMap::Get("Save As"), "Ctrl+A"))
                {
                    isSaveAsClicked = true;
                }
                if (ImGui::MenuItem(WordsMap::Get("Save Scene"), "Ctrl + Alt + A"))
                {
                    SaveScene();
                }
                if (ImGui::MenuItem(WordsMap::Get("Load Scene"), "Ctrl + Alt + L"))
                {
                    LoadScene();
                }
                if (ImGui::MenuItem(WordsMap::Get("Exit"), "Ctrl + W"))
                {
                    Application::App()->Close();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("Edit")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("View")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("Graphics")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("Help")))
            {
                ImGui::EndMenu();
            }
            });

        if (isSaveAsClicked)
        {
            ImGui::OpenPopup(WordsMap::Get("Save Option").c_str());
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        static bool opened = true;
        if (ImGui::BeginPopupModal(WordsMap::Get("Save Option").c_str(), &opened, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(
                "\n"
                "\n\n"
            );
            ImGui::Separator();

            static bool dontAskMeNextTime = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox(WordsMap::Get("Don't ask me next time").c_str(), &dontAskMeNextTime);
            ImGui::PopStyleVar();

            if (ImGui::Button(WordsMap::Get("OK").c_str(), ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button(WordsMap::Get("Cancel").c_str(), ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopFont();
    }

    void UpdateRightClickMenu()
    {
        static ImVec2 pos;
        if (editableArea->IsHovered() && !ImGuizmo::IsOver() &&
            Input::IsMouseButtonPressed(MouseCode::Right) && !Input::IsKeyPressed(KeyCode::Control) && !Input::IsKeyPressed(KeyCode::Alt)
            && !panels.tools.IsControlActive(Tools::Start))
        {
            pos = ImGui::GetMousePos();
            ImGui::OpenPopup("Right Click Menu");
        }
        ImGui::SetNextWindowSize(ImVec2{ 192.0f, 28.0f * 9 });

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::RGBA32(212, 115, 115, 204));
        ImGui::PushStyleColor(ImGuiCol_Separator,     ImGui::RGBA32(119, 119, 119,  88));
        if (ImGui::BeginPopup("Right Click Menu"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::RGBA32(150, 150, 150, 255));
            ImGui::Text("%s", WordsMap::Get("Object Editor").c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();

            if (ImGui::MenuItem(WordsMap::Get("Select/Deselect").c_str()))
            {
                SelectObject(pos.x, pos.y);
            }
            if (ImGui::MenuItem(WordsMap::Get("Import").c_str()))
            {
                LoadObject();
            }
            if (ImGui::MenuItem(WordsMap::Get("Save Scene").c_str()))
            {
                SaveScene();
            }
            if (ImGui::MenuItem(WordsMap::Get("Load Scene").c_str()))
            {
                LoadScene();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu(WordsMap::Get("Create Object").c_str()))
            {
                if (ImGui::MenuItem(WordsMap::Get("Empty")))
                {
                    Object object = scene->CreateObject("Empty");
                }
                if (ImGui::MenuItem(WordsMap::Get("Camera")))
                {
                    Object object = scene->CreateObject("Camera");
                    object.AddComponent<CameraComponent>();
                }
                if (ImGui::MenuItem(WordsMap::Get("Light")))
                {
                    Object object = scene->CreateObject("Light");
                    object.AddComponent<LightComponent>();
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem(WordsMap::Get("Copy").c_str()))
            {
                CopyObject();
            }
            if (ImGui::MenuItem(WordsMap::Get("Paste").c_str()))
            {

            }
            if (ImGui::MenuItem(WordsMap::Get("Duplicate Object").c_str()))
            {
                panels.hierarchyGraphics.Select(CopyObject());
            }
            ImGui::Separator();

            if (ImGui::MenuItem(WordsMap::Get("Delete").c_str()) && selectedObject)
            {
                panels.hierarchyGraphics.Select(Object{});
                scene->DestroyObject(selectedObject);
                selectedObject = Object{};
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
    }

    virtual void OnGuiRender() override
    {
        UpdateEditableArea();
        UpdateMenuBar();
        UpdateRightClickMenu();

        panels
            .hierarchyGraphics
            .OnUpdate(scene, [&](Object &o) -> void {
                selectedObject = o;
            });

        panels
            .navigator
            .OnUpdate(selectedObject, [&]() -> void {
                OnTextureLoaded();
            });

        panels
            .propertyManager
            .OnUpdate(selectedObject);

        panels
            .tools
            .OnUpdate(selectedObject);

        if (panels.tools.IsToolActive(Tools::Move))
        {
            guizmoType = ImGuizmo::OPERATION::TRANSLATE;
        }
        else if (panels.tools.IsToolActive(Tools::Rotate))
        {
            guizmoType = ImGuizmo::OPERATION::ROTATE;
        }
        else if (panels.tools.IsToolActive(Tools::Scale))
        {
            guizmoType = ImGuizmo::OPERATION::SCALE;
        }
        else
        {
            guizmoType = ImGuizmo::OPERATION::INVALID;
        }

        if (Settings.showDemoWindow)
        {
            ImGui::ShowDemoWindow(&Settings.showDemoWindow);
        }
            
        scene->OnGuiRender();

        Application::App()->GetGuiLayer()->BlockEvent(false);
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
        panels.hierarchyGraphics.Select(pixel == -1 || o == selectedObject ? Object{} : o);
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

            panels.hierarchyGraphics.Select(object);

            if (FileSystem::Is3DModel(filepath))
            {
                auto &mesh = object.Add<MeshComponent>();
                mesh.Mesh = std::shared_ptr<Mesh>{ new Mesh{ res.value() } };

                auto &material = object.Add<MaterialComponent>();
                material.References.resize(mesh.Mesh->NodeList().size());
            }
            else if (FileSystem::IsVideo(filepath))
            {
                Ref<Vision::Interface::Demuxer> demuxer;
                Ref<Vision::VideoCodec> decoder;
                if (FileSystem::IsFormat<FileFormat::IVF>(filepath))
                {
                    demuxer = new Vision::IVFDemuxer{};
                    decoder = new Vision::DAV1DCodec{};
                }
                else
                {
                    demuxer = new Vision::FFDemuxer{};
                    decoder = new Vision::FFCodec{};
                }

                demuxer->Open(filepath, decoder);
                Vision::CodedFrame codedFrame;

                auto &videoPlayer = object.AddComponent<VideoPlayerComponent>(decoder, demuxer);

                Vision::Picture picture{};
                do 
                {
                    picture = videoPlayer.GetPicture();
                } while (!picture.Available());
                videoPlayer.PopPicture();

                auto &sprite = object.AddComponent<SpriteRendererComponent>();
                sprite.Sprite = Render::Create<Texture>(picture.desc.width, picture.desc.height, picture.data.get(), desc);
                sprite.Result = Render::Create<Texture>(sprite.Sprite->Width(), sprite.Sprite->Height(), nullptr, desc);

                object.AddComponent<ColorMixingComponent>();
                auto &transform = object.Get<TransformComponent>();
                transform.Scale = Vector3{ sprite.Sprite->Ratio(), 1.0f, 1.0f };
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
        if (panels.tools.IsControlActive(Tools::Start))
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
            panels.tools.Activate(Tools::Hand);
            guizmoType = ImGuizmo::OPERATION::INVALID;
            break;

        case KeyCode::W:
            guizmoType = ImGuizmo::OPERATION::TRANSLATE;
            panels.tools.Activate(Tools::Move);
            if (control || shift)
            {
                Application::App()->Close();
            }
            break;

        case KeyCode::E:
            panels.tools.Activate(Tools::Rotate);
            guizmoType = ImGuizmo::OPERATION::ROTATE;
            break;

        case KeyCode::R:
            panels.tools.Activate(Tools::Scale);
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
        if (editableArea->IsHovered() && !ImGuizmo::IsOver())
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
        if (editableArea->IsHovered())
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
    std::shared_ptr<RenderTarget> renderTarget;

    std::shared_ptr<Shader> shader;

    Vector2 viewportSize;

    Object selectedObject;

    struct {
        std::shared_ptr<GraphicsPipeline> graphics;
    } pipelines;

    struct {
        Camera *primary;

        EditorCamera editor;

        OrthographicCamera orthographic;

        TransformComponent transform;
    } camera;

    struct {
        Navigator navigator;
        HierarchyGraphics hierarchyGraphics;
        PropertyManager propertyManager;
        Tools tools;
    } panels;

    struct {
        bool showDemoWindow{ true };
    } Settings;

    Ref<WFrame> viewport;

    Ref<WImage> editableArea;

    MenuBar menuBar;

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
