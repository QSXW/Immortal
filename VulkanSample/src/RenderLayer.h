#pragma once

#include <Immortal.h>


namespace Immortal
{

class RenderLayer : public Layer
{
public:
    RenderLayer(Vector2 viewport, const std::string &label) :
        Layer(label)
    {
        renderTarget = Render::Create<Framebuffer>(Framebuffer::Description{ viewport, { {  Format::RGBA8 }, { Format::Depth } } });
    }

    virtual void OnDetach() override
    {

    }

    virtual void OnUpdate() override
    {
        Render::Begin(renderTarget, primaryCamera);
        {

        }
        Render::End();
    }

private:
    std::shared_ptr<Framebuffer> renderTarget;

    Camera primaryCamera;
};

}
