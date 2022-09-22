#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "DescriptorHeap.h"
#include "CommandList.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"

namespace Immortal
{
namespace D3D12
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

protected:
	void __CreateFontsTexture();

    void __RenderDrawData(CommandList *commandList);

private:
    RenderContext *context{ nullptr };

    Swapchain *swapchain{ nullptr };

    DescriptorHeap *srvDescriptorHeap{ nullptr };

    URef<GraphicsPipeline> pipeline;

    URef<Texture> fonts;

    struct {
        Ref<Buffer> vertex;
        Ref<Buffer> index;
	} buffers[3];

    uint32_t sync = 0;

public:
    Vector4 clearColor{ .4f, .5f, .6f, 1.0f };
};

}
}
