#pragma once

#include <Immortal.h>
#include "Platform/Vulkan/Common.h"
#include "Platform/Vulkan/Device.h"

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
            { {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
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

        camera.transform.Position = Vector3{ .5f, 0.0, 0.0 };
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

        ubo.projectionMatrix = camera.primaryCamera.Projection();
        ubo.viewMatrix       = camera.primaryCamera.View();
        ubo.modelMatrix      = camera.transform;
        uniformBuffer->Update(sizeof(ubo), &ubo);

        Render::Begin(renderTarget, camera.primaryCamera);
        {
            Render::Draw(pipeline);
        }
        Render::End();
    }

    virtual void OnGuiRender() override
    {
        ImGui::Begin("Render Target");
        ImGui::ColorEdit4("Clear Color", rcast<float *>(renderTarget->ClearColor()));
        ImGui::End();
        offline.OnUpdate(renderTarget);
    }

private:
    std::shared_ptr<RenderTarget> renderTarget;

    std::shared_ptr<Shader> shader;

    std::shared_ptr<Pipeline> pipeline;

    struct {
        SceneCamera primaryCamera;

        TransformComponent transform;
    } camera;

    Widget::Viewport offline{ "Offline Render" };

    struct
    {
        Matrix4 projectionMatrix;
        Matrix4 modelMatrix;
        Matrix4 viewMatrix;
    } ubo;

    std::shared_ptr<Buffer> uniformBuffer;

    Entity cameraObject;
};

}
