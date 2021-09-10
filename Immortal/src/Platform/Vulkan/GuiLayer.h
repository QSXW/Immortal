#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

class GuiLayer : virtual public Immortal::GuiLayer
{
public:
    using Super = Immortal::GuiLayer;

public:
    GuiLayer(RenderContext *context);

    ~GuiLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event &e) override;
    virtual void OnGuiRender() override;

    virtual void Begin() override;
    virtual void End() override;

private:
    RenderContext *context{ nullptr };
};

}
}
