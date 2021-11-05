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
class Texture;
class Framebuffer : public SuperFramebuffer
{
public:
    using Super = SuperFramebuffer;

    struct Attachment
    {
        std::unique_ptr<Image> image;
        std::unique_ptr<ImageView> view;
    };

public:
    Framebuffer();

    Framebuffer(Device *device, const Super::Description &spec);

    Framebuffer(Device *device, std::shared_ptr<RenderPass> &renderPass, std::vector<ImageView> &views, VkExtent2D &extent);

    ~Framebuffer();

    void INIT(std::vector<VkImageView> views);

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

private:
    VkFramebuffer handle{ VK_NULL_HANDLE };

    Device *device{ nullptr };

    std::shared_ptr<RenderPass> renderPass{ nullptr };

    struct
    {
        Attachment depth;
        std::vector<Attachment> colors;
    } attachments;
};
}
}
