#pragma once

#include "Render/RenderTarget.h"

#include "Common.h"
#include "Device.h"
#include "Attachment.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"
#include "Submitter.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace Vulkan
{

struct CompareExtent2D
{
    bool operator()(const VkExtent2D &lhs, const VkExtent2D &rhs) const
    {
        return !(lhs.width == rhs.width && lhs.height == rhs.height) && (lhs.width < rhs.width &&lhs.height < rhs.height);
    }
};

class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget();

    RenderTarget(Device *device, const Description &description);

    RenderTarget(RenderTarget &&other);

    ~RenderTarget();

    RenderTarget &operator=(RenderTarget &&other);

    void Invalidate(Image &&image, Ref<RenderPass> renderPass);

    void InvalidateFrameBuffer();

    RenderTarget(const RenderTarget &) = delete;
    RenderTarget &operator=(const RenderTarget &) = delete;

    template <class T>
    requires std::is_same_v<T, RenderPass> || std::is_same_v<T, Framebuffer>
    T &Get()
    {
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return *renderPass;
        }
        if constexpr (IsPrimitiveOf<Framebuffer, T>())
        {
            return framebuffer;
        }
    }

    template <class T>
    requires std::is_same_v<T, RenderPass> || std::is_same_v<T, Framebuffer>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return renderPass;
        }
        if constexpr (IsPrimitiveOf<Framebuffer, T>())
        {
            return &framebuffer;
        }
    }

    VkRenderPass GetRenderPass() const
    {
        return *renderPass;
    }

    VkFramebuffer GetFramebuffer()
    {
        return framebuffer;
    }

    uint32_t ColorAttachmentCount() const
    {
        return attachments.colors.size();
    }

    const Image &GetColorImage(uint32_t index = 0) const
    {
        return attachments.colors[index].image;
    }

    void Create();

    void SetupDescriptor();

public:
    virtual operator uint64_t() const override;

    virtual void Resize(uint32_t x, uint32_t y) override;

    virtual uint64_t PickPixel(uint32_t index, uint32_t x, uint32_t y, Format format) override;

    virtual void Map(uint32_t index, uint8_t **ppData) override;
                 
    virtual void Unmap(uint32_t index) override;

private:
    Device *device{ nullptr };

    Ref<RenderPass> renderPass;

    Framebuffer framebuffer;

    DescriptorSet descriptorSet;

    Sampler sampler;

    struct
    {
        Attachment depth;

        LightArray<Attachment, 3> colors;
    } attachments;

    std::vector<std::unique_ptr<Image>> stagingImages;
};

}
}
