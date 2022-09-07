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
    void __InvalidateRenderBuffer(size_t size)
    {
        auto device = context->GetAddress<Device>();
        if constexpr (T == Buffer::Type::Vertex)
        {
            buffers.vertex = new Buffer{device, size, Buffer::Type::Vertex};
        }
        else if constexpr (T == Buffer::Type::Index)
        {
            buffers.index = new Buffer{device, size, Buffer::Type::Index};
        }
    }

private:
    RenderContext *context{ nullptr };

    std::unique_ptr<DescriptorPool> descriptorPool;

    Ref<GraphicsPipeline> pipeline;

    Ref<Shader> shader;

    Ref<Texture> fonts;

    struct {
        Ref<Buffer> vertex;
        Ref<Buffer> index;
    } buffers;
};

}
}
