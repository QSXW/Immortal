#pragma once

#include <Immortal.h>
#include "Panel/Navigator.h"
#include "Panel/HierarchyGraphics.h"
#include "Panel/PropertyManager.h"
#include "Panel/Tools.h"

namespace Immortal
{

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer{ label },
        eventSink{ this },
        selectedObject{},
        editorCamera{ Vector::PerspectiveFOV(Vector::Radians(90.0f), viewport.x, viewport.y, 0.1f, 1000.0f) }
    {
        eventSink.Listen(&RenderLayer::OnKeyPressed, Event::Type::KeyPressed);
        eventSink.Listen(&RenderLayer::OnMouseDown, Event::Type::MouseButtonPressed);

        renderTarget.reset(Render::Create<RenderTarget>(RenderTarget::Description{ viewport, { {  Format::RGBA8, Texture::Wrap::Clamp, Texture::Filter::Bilinear }, { Format::Depth } } }));

        shader = Render::Get<Shader, ShaderName::Basic>();

        uniformBuffer.reset(Render::Create<Buffer>(sizeof(ubo), 0));

        pipelines.graphics.reset(Render::Create<Pipeline::Graphics>(shader));

        auto &triangle = DataSet::Classic::Triangle;
        pipelines.graphics->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(triangle.Vertices(), Buffer::Type::Vertex) });
        pipelines.graphics->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(triangle.Indices(), Buffer::Type::Index)   });
        pipelines.graphics->Set(triangle.Description());
        pipelines.graphics->Create(renderTarget);
        pipelines.graphics->Bind(Render::Preset()->WhiteTexture.get(), 1);
        pipelines.graphics->Bind("UBO", uniformBuffer.get());

        camera.primaryCamera.SetOrthographic(1.2f);
        camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());

        camera.transform.Position = Vector3{ 0.0f, 0.0, -1.0f };
    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
        auto pos = Input::GetMousePosition();

        Vector2 viewportSize = editableArea.Size();
        if ((viewportSize.x != renderTarget->Width() || viewportSize.y != renderTarget->Height()) &&
            (viewportSize.x != 0 && viewportSize.y != 0))
        {
            renderTarget->Resize(viewportSize);
            editorCamera.SetViewportSize(viewportSize);
            camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());
            scene.SetViewportSize(viewportSize);
        }

        if (panels.tools.IsControlActive(Tools::Start))
        {
            scene.OnRenderRuntime();
        }
        else
        {
            if (editableArea.IsHovered())
            {
                editorCamera.OnUpdate();
            }
            scene.OnRenderEditor(editorCamera);
        }
    }

    void UpdateEditableArea()
    {
        editableArea.OnUpdate(scene.Target(), [&]() -> void {

            if (selectedObject && guizmoType != ImGuizmo::OPERATION::INVALID)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

                Matrix4 cameraProjectionMatrix = editorCamera.Projection();
                Matrix4 cameraViewMatrix = editorCamera.View();

                TransformComponent &transform = selectedObject.GetComponent<TransformComponent>();
                Matrix4 munipulatedTransform = transform.Transform();

                bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
                float snapValues[][3] = {
                    {  0.5f,  0.5f,  0.5f },
                    { 45.0f, 45.0f, 45.0f },
                    {  0.5f,  0.5f,  0.5f }
                };

                ImGuizmo::Manipulate(
                    &cameraViewMatrix[0].x,
                    &cameraProjectionMatrix[0].x,
                    static_cast<ImGuizmo::OPERATION>(guizmoType),
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

    void UpdateMenuBar()
    {
        auto *gui = Application::App()->GetGuiLayer();
        gui->UpdateTheme();

        ImGui::PushFont(gui->Bold());
        menuBar.OnUpdate([=]() -> void {
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
        ImGui::PopFont();
    }

    virtual void OnGuiRender() override
    {
        UpdateEditableArea();
        UpdateMenuBar();

        panels
            .hierarchyGraphics
            .OnUpdate(&scene, [&](Object &o) -> void {
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

        Application::App()->GetGuiLayer()->BlockEvent(false);
    }

    void OnTextureLoaded()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            std::shared_ptr<Texture> newTexture{ Render::Create<Texture>(res.value()) };
            auto &transform = selectedObject.GetComponent<TransformComponent>();
            transform.Scale = transform.Scale.z * Vector3{ newTexture->Ratio(), 1.0f, 1.0f };

            if (!selectedObject.Has<SpriteRendererComponent>())
            {
                selectedObject.Add<SpriteRendererComponent>();
            }
            auto &sprite = selectedObject.Get<SpriteRendererComponent>();
            sprite.Texture = newTexture;
            sprite.Final.reset(Render::Create<Texture>(sprite.Texture->Width(), sprite.Texture->Height(), nullptr, Texture::Description{
                Format::RGBA8,
                Texture::Wrap::Clamp
                }));
        }        
    }

    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            auto o = scene.CreateObject(res.value());

            panels.hierarchyGraphics.Select(o);

            if (FileSystem::IsFormat<FileFormat::OBJ>(res.value()) || FileSystem::IsFormat<FileFormat::FBX>(res.value()))
            {
                auto &mesh = o.Add<MeshComponent>();
                mesh.Mesh = std::shared_ptr<Mesh>{ new Mesh{ res.value() } };

                auto &matrial = o.Add<MaterialComponent>();
            }
            else 
            {
                std::shared_ptr<Texture> texture{ Render::Create<Texture>(res.value()) };
                auto &sprite = o.Add<SpriteRendererComponent>();
                sprite.Texture = texture;
                sprite.Final.reset(Render::Create<Texture>(texture->Width(), texture->Height(), nullptr, Texture::Description{
                    Format::RGBA8,
                    Texture::Wrap::Clamp
                    }));

                o.Add<ColorMixingComponent>();
     
                auto &transform = o.Get<TransformComponent>();
                transform.Scale = Vector3{ texture->Ratio(), 1.0f, 1.0f };
            }

            return true;
        }

        return false;
    }

    bool OnKeyPressed(KeyPressedEvent &e)
    {
        if (e.RepeatCount() > 0)
        {
            return false;
        }

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool shift   = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
        switch (e.GetKeyCode())
        {
        case KeyCode::L:
            if (control || shift)
            {
                LoadObject();
            }
            break;

        case KeyCode::S:
            if (control)
            {
                scene.Target()->PickPixel(0, 0, 0, Format::RGBA8);
                Async::Execute([&]() -> void {
                    auto size = editableArea.Size();
                    uint32_t width = U32(size.x);
                    uint32_t height = U32(size.y);

                    uint8_t *dataMapped = nullptr;
                    scene.Target()->Map(0, &dataMapped);

                    Media::BMPCodec bmp{};
                    bmp.Write("RenderTarget.bmp", width, height, 4, dataMapped, (SLALIGN(width, 8) - width) * 4);

                    scene.Target()->Unmap(0);
                });
            }
            break;

        case KeyCode::F:
            if (!!selectedObject)
            {
                editorCamera.Focus(selectedObject.GetComponent<TransformComponent>().Position);
            } 
            break;

        case KeyCode::Q:
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
        if (editableArea.IsHovered() && !ImGuizmo::IsOver() && e.GetMouseButton() == MouseCode::Left)
        {
            if (!Input::IsKeyPressed(KeyCode::Control))
            {
                return true;
            }
            auto [x, y] = ImGui::GetMousePos();
            x -= editableArea.MinBound().x;
            y -= editableArea.MinBound().y;

            uint64_t pixel = scene.Target()->PickPixel(1, x, y, Format::R32);

            Object o = Object{ *(int *)&pixel, &scene };
            panels.hierarchyGraphics.Select(o == selectedObject ? Object{} : o);

            return true;
        }

        return false;
    }

    virtual void OnEvent(Event &e) override
    {
        eventSink.Dispatch(e);
    }

private:
    std::shared_ptr<RenderTarget> renderTarget;

    std::shared_ptr<Shader> shader;


    Object selectedObject;

    struct {
        std::shared_ptr<GraphicsPipeline> graphics;
    } pipelines;

    struct {
        SceneCamera primaryCamera;

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

    Widget::Viewport editableArea{ WordsMap::Get("Offline Render") };
    Widget::MenuBar menuBar;

    struct
    {
        Matrix4 viewProjection;
        Matrix4 modeTransform;
    } ubo;

    std::shared_ptr<Buffer> uniformBuffer;

    Object cameraObject;

    EventSink<RenderLayer> eventSink;

    Scene scene{ "Viewport", true };

    Object triangle;

    EditorCamera editorCamera;

    int guizmoType = ImGuizmo::OPERATION::INVALID;
};

}
