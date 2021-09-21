#pragma once

#include "Render/Framebuffer.h"

#include "Common.h"

#include "Image.h"
#include "ImageView.h"

namespace Immortal
{
namespace Vulkan
{

class RenderContext;
class Device;
class RenderPass;

class Framebuffer : public SuperFramebuffer
{
public:
    using Super = SuperFramebuffer;

public:
    Framebuffer();

    Framebuffer(const Super::Description &spec);

    Framebuffer(Device *device, RenderPass *renderPass, std::vector<ImageView> &views, VkExtent2D &extent);

    ~Framebuffer();

    void INIT();

    VkFramebuffer &Handle()
    {
        return handle;
    }

    virtual void Map() override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual void *ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width, int height) override;

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

    virtual UINT32 ColorAttachmentRendererID(UINT32 index) const override;

    virtual UINT32 DepthAttachmentRendererID(UINT32 index) const override;

    virtual const Super::Description &Desc() const override;

private:
    VkFramebuffer handle{ VK_NULL_HANDLE };

    RenderContext *context{ nullptr };

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
