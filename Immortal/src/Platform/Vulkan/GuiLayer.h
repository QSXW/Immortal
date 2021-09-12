#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "Common.h"
#include "DescriptorPool.h"

namespace Immortal
{
namespace Vulkan
{

class GuiLayer : virtual public Immortal::GuiLayer
{
public:
    using Super = Immortal::GuiLayer;

public:
    GuiLayer(RenderContext *context, VkRenderPass renderPass);

    ~GuiLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event &e) override;
    virtual void OnGuiRender() override;

    virtual void Begin() override;
    virtual void End() override;

private:
    RenderContext *context{ nullptr };

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

    VkRenderPass renderPass{ VK_NULL_HANDLE };
};

}
}
