#pragma once

#include <Immortal.h>

namespace Immortal
{

struct Vertex
{
    Vector3 pos;
    Vector3 color;
};

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer(label)
    {
        renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ viewport, { {  Format::RGBA8, Texture::Wrap::Clamp, Texture::Filter::Bilinear }, { Format::Depth } } });

        const std::vector<Vertex> vertices = {
            { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2
        };

        shader = Render::Create<Shader>("Assets\\shaders\\glsl\\basic");

        pipeline = Render::Create<Pipeline>(shader);

        pipeline->Set(Render::Create<Buffer>(vertices.size(), vertices.data(), Buffer::Type::Vertex));
        pipeline->Set(Render::Create<Buffer>(indices.size(), indices.data(), Buffer::Type::Index));
        pipeline->Set({
            { Format::VECTOR3, "Position" },
            { Format::VECTOR3, "Color"    }
            });
        pipeline->Create(renderTarget);
        pipeline->Bind(Render::Preset()->WhiteTexture, 1);

        camera.primaryCamera.SetPerspective(90.0f);
        camera.primaryCamera.SetViewportSize(renderTarget->ViewportSize());

        uniformBuffer = Render::Create<Buffer>(sizeof(ubo), Buffer::Type::Uniform);
        pipeline->Bind("ubo", uniformBuffer.get());

        camera.transform.Position = Vector3{ 0.0f, 0.0, -1.0f };

        auto clearColor = renderTarget->ClearColor();
        clearColor->r = 0.10980392;
        clearColor->g = 0.10980392;
        clearColor->b = 0.10980392;

        Render2D::Setup(renderTarget);
    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
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
        ImGui::End();
        offline.OnUpdate(renderTarget);
    }

    void OnTextureLoaded(const std::string &path)
    {
        image = Render::Create<Texture>(path);
    }

private:
    std::shared_ptr<RenderTarget> renderTarget;

    std::shared_ptr<Shader> shader;

    std::shared_ptr<Pipeline> pipeline;

    struct {
        SceneCamera primaryCamera;

        TransformComponent transform;
    } camera;

    Widget::Viewport offline{ WordsMap::Get("offlineRender")};

    struct
    {
        Matrix4 viewProjection;
        Matrix4 modeTransform;
    } ubo;

    std::shared_ptr<Buffer> uniformBuffer;

    Entity cameraObject;

    std::shared_ptr<Texture> image{ Render::Preset()->WhiteTexture };
};

}
