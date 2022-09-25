#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"

namespace Immortal
{
namespace D3D11
{

class Pipeline;
class GuiLayer : virtual public SuperGuiLayer
{
public:
    using Super = SuperGuiLayer;

public:
    GuiLayer(SuperRenderContext *context);

    virtual ~GuiLayer();

    virtual void OnAttach() override;

    virtual void OnDetach() override { }

    virtual void OnEvent(Event &e) override;

    virtual void Begin() override;
    virtual void End() override;

private:
    RenderContext *context{ nullptr };

public:
    Vector4 clearColor{ .4f, .5f, .6f, 1.0f };
};

}
}
