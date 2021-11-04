#pragma once

#include "Render/Framebuffer.h"

#include "Common.h"
#include "RenderPass.h"
#include "Image.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class RenderPass;

class Framebuffer : public SuperFramebuffer
{
public:
    using Super = SuperFramebuffer;

public:
    Framebuffer();

    Framebuffer(Device *device, const Super::Description &spec);

    Framebuffer(Device *device, RenderPass *renderPass, std::vector<ImageView> &views, VkExtent2D &extent);

    ~Framebuffer();

    void INIT();

    VkFramebuffer &Handle()
    {
        return handle;
    }

    operator VkFramebuffer&()
    {
        return handle;
    }

    operator VkFramebuffer() const
    {
        return handle;
    }

    template <class T = RenderPass>
    T Get()
    {
        static_assert(is_same<T, RenderPass>() && "Only RenderPass can be got!");
        return *renderPass;
    }

public SLVIRTUAL:
    virtual void Map() override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual void *ReadPixel(UINT32 attachmentIndex, int x, int y, Format format, int width, int height) override;

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

    virtual UINT32 ColorAttachmentHandle(UINT32 index) const override;

    virtual UINT32 DepthAttachmentHandle(UINT32 index) const override;

private:
    VkFramebuffer handle{ VK_NULL_HANDLE };

    Device *device{ nullptr };

    RenderPass *renderPass{ nullptr };

    struct
    {
        std::unique_ptr<Image> image;
        std::unique_ptr<ImageView> view;
    } depthStencil;
};
}
}
