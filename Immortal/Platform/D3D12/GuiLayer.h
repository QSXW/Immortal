#pragma once

#include "ImGui/GuiLayer.h"
#include "RenderContext.h"
#include "DescriptorHeap.h"
#include "CommandList.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include "Algorithm/Rotate.h"

namespace Immortal
{
namespace D3D12
{

struct RenderBuffer
{
	Ref<Buffer> vertex;
	Ref<Buffer> index;
};

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

    URef<GraphicsPipeline> pipeline;

    URef<Texture> fonts;

    Rotate<RenderBuffer, 4> buffers;

public:
    Vector4 clearColor{ .4f, .5f, .6f, 1.0f };
};

}
}
