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

        renderTarget.reset(Render::Create<RenderTarget>(RenderTarget::Description{ viewport, { {  Format::RGBA8, Texture::Wrap::Clamp, Texture::Filter::Bilinear }, { Format::Depth } } }));

        shader = Render::Get<Shader, ShaderName::Basic>();

        pipeline.reset(Render::Create<Pipeline>(shader));

        auto &triangle = DataSet::Classic::Triangle;

        pipeline->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(triangle.Vertices(), Buffer::Type::Vertex) });
        pipeline->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(triangle.Indices(), Buffer::Type::Index)   });
        pipeline->Set(triangle.Description());
        pipeline->Create(renderTarget);
        pipeline->Bind(Render::Preset()->WhiteTexture, 1);

        // camera.primaryCamera.SetPerspective(90.0f);
        camera.primaryCamera.SetOrthographic(1.2f);
        camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());

        uniformBuffer.reset(Render::Create<Buffer>(sizeof(ubo), 0));
        pipeline->Bind("UBO", uniformBuffer.get());

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
        editorCamera.OnUpdate();

        scene.OnRenderEditor(editorCamera);
    }

    void UpdateEditableArea()
    {
        editableArea.OnUpdate(scene.Target() , [&]() -> void {
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
            image.reset(Render::Create<Texture>(res.value()));
            auto &transform = selectedObject.GetComponent<TransformComponent>();
            transform.Scale = transform.Scale.z * Vector3{ image->Ratio(), 1.0f, 1.0f };

            if (!selectedObject.Has<SpriteRendererComponent>())
            {
                selectedObject.Add<SpriteRendererComponent>();
            }
            selectedObject.Get<SpriteRendererComponent>().Texture = image;
        }        
    }

    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            auto o = scene.CreateObject(res.value());

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

    virtual void OnEvent(Event &e) override
    {
        eventSink.Dispatch(e);
    }

private:
    std::shared_ptr<RenderTarget> renderTarget;

    std::shared_ptr<Shader> shader;

    std::shared_ptr<Pipeline> pipeline;

    Object selectedObject;

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

    std::shared_ptr<Texture> image{ Render::Preset()->WhiteTexture };

    EventSink<RenderLayer> eventSink;

    Scene scene{ "Viewport", true };

    Object triangle;

    EditorCamera editorCamera;

    int guizmoType = ImGuizmo::OPERATION::INVALID;
};

}
