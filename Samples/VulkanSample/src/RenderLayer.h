#pragma once

#include <Immortal.h>
#include "Panel/Navigator.h"

namespace Immortal
{

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer{ label },
        eventSink{ this },
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

        renderTarget->Set(Color{ 0.10980392f, 0.10980392f, 0.10980392f, 1 });

        Render2D::Setup(renderTarget);

        selectedObject = scene.CreateObject("Texture");
        selectedObject.Add<SpriteRendererComponent>();
        auto &transform = selectedObject.GetComponent<TransformComponent>();
        transform.Scale = Vector3{ 8.0f, 8.0f, 8.0f };
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
        }
        editorCamera.OnUpdate();

        auto &transform = selectedObject.GetComponent<TransformComponent>();

        ubo.viewProjection = editorCamera.ViewProjection();
        ubo.modeTransform  = transform;
        uniformBuffer->Update(sizeof(ubo), &ubo);

        Render::Begin(renderTarget, editorCamera);
        {
            // Render::Draw(pipeline);
            Render2D::BeginScene(editorCamera);
            Render2D::DrawQuad(transform, image);
            Render2D::EndScene();
        }
        Render::End();
    }

    void UpdateEditableArea()
    {
        editableArea.OnUpdate(renderTarget, [&]() -> void {
            if (guizmoType != ImGuizmo::OPERATION::INVALID)
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
            if (ImGui::BeginMenu(WordsMap::Get("menu")))
            {
                if (ImGui::MenuItem(WordsMap::Get("open"), "Ctrl + O"))
                {
                    this->LoadObject();
                }
                if (ImGui::MenuItem(WordsMap::Get("save"), "Ctrl + S"))
                {

                }
                if (ImGui::MenuItem(WordsMap::Get("saveAs"), "Ctrl+A"))
                {

                }
                if (ImGui::MenuItem(WordsMap::Get("exit"), "Ctrl + W"))
                {
                    Application::App()->Close();
                }
                if (ImGui::MenuItem(WordsMap::Get("open"), "Ctrl + W"))
                {

                }
                if (ImGui::MenuItem(WordsMap::Get("loadObject"), "Ctrl + L"))
                {
                    auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
                    if (res.has_value())
                    {
                        OnTextureLoaded(res.value());
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("edit")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("view")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("graphics")))
            {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(WordsMap::Get("help")))
            {
                ImGui::EndMenu();
            }
            });
        ImGui::PopFont();
    }

    virtual void OnGuiRender() override
    {
        auto &transform = selectedObject.GetComponent<TransformComponent>();
        ImGui::Begin("Render Target");
        ImGui::ColorEdit4("Clear Color", rcast<float *>(renderTarget->ClearColor()));
        UI::DrawVec3Control("Position", transform.Position);
        UI::DrawVec3Control("Rotation", transform.Rotation);
        UI::DrawVec3Control("Scale", transform.Scale);
        if (ImGui::DragFloat("Orthographic Size", &camera.primaryCamera.OrthographicSize(), 0.1f, 0.000001f, 90.0f))
        {
            camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());
        }
        ImGui::End();

        UpdateEditableArea();
        UpdateMenuBar();
        panels.navigator.OnUpdate(selectedObject, [&]() -> void { LoadObject(); });

        if (Settings.showDemoWindow)
        {
            ImGui::ShowDemoWindow(&Settings.showDemoWindow);
        }

        Application::App()->GetGuiLayer()->BlockEvent(false);
    }

    void OnTextureLoaded(const std::string &path)
    {
        image.reset(Render::Create<Texture>(path));
        auto &transform = selectedObject.GetComponent<TransformComponent>();
        transform.Scale = transform.Scale.z * Vector3{ image->Ratio(), 1.0f, 1.0f };
        selectedObject.Get<SpriteRendererComponent>().Texture = image;
    }

    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            image.reset(Render::Create<Texture>(res.value()));
            auto &transform = selectedObject.GetComponent<TransformComponent>();
            transform.Scale = transform.Scale.z * Vector3{ image->Ratio(), 1.0f, 1.0f };
            selectedObject.Get<SpriteRendererComponent>().Texture = image;
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
            editorCamera.Focus(selectedObject.GetComponent<TransformComponent>().Position);
            break;

        case KeyCode::Q:
            guizmoType = ImGuizmo::OPERATION::INVALID;
            break;

        case KeyCode::W:
            guizmoType = ImGuizmo::OPERATION::TRANSLATE;
            if (control || shift)
            {
                Application::App()->Close();
            }
            break;

        case KeyCode::E:
            guizmoType = ImGuizmo::OPERATION::ROTATE;
            break;

        case KeyCode::R:
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

    struct {
        SceneCamera primaryCamera;

        TransformComponent transform;
    } camera;

    struct {
        Navigator navigator;
    } panels;

    struct {
        bool showDemoWindow{ true };
    } Settings;

    Widget::Viewport editableArea{ WordsMap::Get("offlineRender") };
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

    Object selectedObject;

    Object triangle;

    EditorCamera editorCamera;

    int guizmoType = ImGuizmo::OPERATION::INVALID;
};

}
