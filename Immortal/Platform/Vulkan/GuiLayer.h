#pragma once

#include "ImGui/GuiLayer.h"
#include "Common.h"
#include "DescriptorPool.h"

namespace Immortal
{
namespace Vulkan
{

class RenderContext;
class GuiLayer : virtual public SuperGuiLayer
{
public:
    using Super = SuperGuiLayer;

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
    void __RenderDrawData();

private:
    RenderContext *context{ nullptr };

    std::unique_ptr<DescriptorPool> descriptorPool;
};

}
}
