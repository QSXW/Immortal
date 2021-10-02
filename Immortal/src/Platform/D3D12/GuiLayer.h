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
    GuiLayer(SuperRenderContext *context);

    ~GuiLayer() { }

    virtual void OnAttach() override;

    virtual void OnDetach() override { }

    virtual void OnEvent(Event &e) override;

    virtual void OnGuiRender() override;

    virtual void Begin() override;
    virtual void End() override;

private:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    DescriptorPool *srvDescriptorHeap{ nullptr };

    CommandAllocator *commandAllocator{ nullptr };

    CommandList *commandList{ nullptr };

    Queue *queue{ nullptr };

public:
    Vector4 clearColor{ .4f, .5f, .6f, 1.0f };
};

}
}
