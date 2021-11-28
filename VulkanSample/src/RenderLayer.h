#pragma once

#include <Immortal.h>


namespace Immortal
{

struct Vertex
{
    Vector2 pos;
    Vector3 color;
};

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer(label)
    {
        renderTarget = Render::Create<RenderTarget>(RenderTarget::Description{ viewport, { {  Format::RGBA8 }, { Format::Depth } } });

        const std::vector<Vertex> vertices = {
            {{  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
            {{  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }},
            {{ -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }}
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        shader = Render::Create<Shader>("Assets\\shaders\\glsl\\basic");

        pipeline = Render::Create<Pipeline>(shader);

        pipeline->Set(Render::Create<Buffer>(vertices.size(), vertices.data(), Buffer::Type::Vertex));
        pipeline->Set(Render::Create<Buffer>(indices.size(), indices.data(), Buffer::Type::Index));
        pipeline->Set({
            { Format::VECTOR2, "Position" },
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
        Render::Begin(renderTarget, primaryCamera);
        {
            Render::Draw(pipeline);
        }
        Render::End();
    }

    virtual void OnGuiRender() override
    {
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
