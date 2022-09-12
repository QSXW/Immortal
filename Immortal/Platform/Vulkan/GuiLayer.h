#pragma once

#include "ImGui/GuiLayer.h"
#include "Buffer.h"
#include "Common.h"
#include "DescriptorPool.h"
#include "Shader.h"
#include "RenderContext.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

class CommandBuffer;
class GraphicsPipeline;
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
    void __CreateFontsTexture();

    void __RenderDrawData(CommandBuffer *cmdbuf);

    template <Buffer::Type T>
    void __InvalidateRenderBuffer(Ref<Buffer> &buffer, size_t size)
    {
        auto device = context->GetAddress<Device>();
	    buffer = new Buffer{device, size, T};
    }

private:
    RenderContext *context{ nullptr };

    URef<DescriptorPool> descriptorPool;

    Ref<GraphicsPipeline> pipeline;

    Ref<Shader> shader;

    Ref<Texture> fonts;

    struct {
        Ref<Buffer> vertex;
        Ref<Buffer> index;
	} buffers[3];

    uint32_t sync = 0;
};

}
}
