#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "Common.h"
#include "DescriptorPool.h"

namespace Immortal
{
namespace Vulkan
{

class GuiLayer : virtual public SuperGuiLayer
{
public:
    using Super = SuperGuiLayer;

public:
    GuiLayer(RenderContext::Super *context);

    ~GuiLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event &e) override;
    virtual void OnGuiRender() override;

    virtual void Begin() override;

    virtual void End() override;

private:
    RenderContext *context{ nullptr };

    Device *device{ nullptr };

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

    RenderPass *renderPass{ nullptr };
    
    uint32_t frameIndex{ 0 };
};

}
}
