#pragma once

#include <Immortal.h>

namespace Immortal
{

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer{ label },
        eventSink{ this }
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
    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
        auto pos = Input::GetMousePosition();

        Vector2 viewportSize = offline.Size();
        if ((viewportSize.x != renderTarget->Width() || viewportSize.y != renderTarget->Height()) &&
            (viewportSize.x != 0 && viewportSize.y != 0))
        {
            renderTarget->Resize(viewportSize);
            camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());
        }

        ubo.viewProjection = camera.primaryCamera.ViewProjection();
        ubo.modeTransform  = camera.transform;
        uniformBuffer->Update(sizeof(ubo), &ubo);

        camera.transform.Scale = { image->Ratio(), 1.0, 0.0 };

        Render::Begin(renderTarget, camera.primaryCamera);
        {
            Render::Draw(pipeline);
            Render2D::BeginScene(camera.primaryCamera);
            Render2D::DrawQuad(camera.transform, image);
            Render2D::EndScene();
        }
        Render::End();
    }

    virtual void OnGuiRender() override
    {
        ImGui::Begin("Render Target");
        ImGui::ColorEdit4("Clear Color", rcast<float *>(renderTarget->ClearColor()));
        UI::DrawVec3Control("Position", camera.transform.Position);
        UI::DrawVec3Control("Rotation", camera.transform.Rotation);
        UI::DrawVec3Control("Scale", camera.transform.Scale);
        if (ImGui::DragFloat("Orthographic Size", &camera.primaryCamera.OrthographicSize(), 0.1f, 0.000001f, 90.0f))
        {
            camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());
        }
        ImGui::End();
        offline.OnUpdate(renderTarget);

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

        if (Settings.showDemoWindow)
        {
            ImGui::ShowDemoWindow(&Settings.showDemoWindow);
        }

        Application::App()->GetGuiLayer()->BlockEvent(false);
    }

    void OnTextureLoaded(const std::string &path)
    {
        image.reset(Render::Create<Texture>(path));
    }

    bool LoadObject()
    {
        auto res = FileDialogs::OpenFile(FileDialogs::ImageFilter);
        if (res.has_value())
        {
            image.reset(Render::Create<Texture>(res.value()));
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

        case KeyCode::W:
            if (control || shift)
            {
                Application::App()->Close();
            }
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
        bool showDemoWindow{ true };
    } Settings;

    Widget::Viewport offline{ WordsMap::Get("offlineRender")};
    Widget::MenuBar menuBar;

    struct
    {
        Matrix4 viewProjection;
        Matrix4 modeTransform;
    } ubo;

    std::shared_ptr<Buffer> uniformBuffer;

    Entity cameraObject;

    std::shared_ptr<Texture> image{ Render::Preset()->WhiteTexture };

    EventSink<RenderLayer> eventSink;

    Scene scene{ "Viewport", true };

    GameObject textureObject;
};

}
