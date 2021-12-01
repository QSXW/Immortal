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
    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
        Vector2 viewportSize = offline.Size();
        if ((viewportSize.x != renderTarget->Desc().Width ||
            viewportSize.y != renderTarget->Desc().Height) &&
            (viewportSize.x != 0 && viewportSize.y != 0))
        {
            renderTarget->Resize(viewportSize);
        }
        Render::Begin(renderTarget, primaryCamera);
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

    Camera primaryCamera;

    Widget::Viewport offline{ "Offline Render" };
};

}
