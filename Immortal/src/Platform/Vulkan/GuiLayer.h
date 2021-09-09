#pragma once
#include "ImGui/GuiLayer.h"

namespace Immortal
{
namespace Vulkan
{

class GuiLayer : virtual public Immortal::GuiLayer
{
public:
    using Super = Immortal::GuiLayer;

public:
    GuiLayer();
    ~GuiLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event &e) override;
    virtual void OnGuiRender() override;

    virtual void Begin() override;
    virtual void End() override;
};

}
}
