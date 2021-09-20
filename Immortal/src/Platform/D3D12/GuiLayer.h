#pragma once

#include "ImGui/GuiLayer.h"

#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

class GuiLayer : virtual public SuperGuiLayer
{
public:
    using Super = SuperGuiLayer;

public:
    GuiLayer(RenderContext::Super *context);

    ~GuiLayer() { }

    virtual void OnAttach() override { }
    virtual void OnDetach() override { }
    virtual void OnEvent(Event &e) override { }
    virtual void OnGuiRender() override { }

    virtual void Begin() override { }
    virtual void End() override { }

private:
    
};

}
}
