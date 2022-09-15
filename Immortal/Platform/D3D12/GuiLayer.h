#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

namespace Immortal
{
namespace D3D12
{

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

    virtual void OnGuiRender() override;

    virtual void Begin() override;
    virtual void End() override;

private:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    DescriptorHeap *srvDescriptorHeap{ nullptr };

public:
    Vector4 clearColor{ .4f, .5f, .6f, 1.0f };
};

}
}
