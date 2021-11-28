#pragma once

#include "Render/RenderTarget.h"

#include "Common.h"
#include "Device.h"
#include "Attachment.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"

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

    static std::unique_ptr<RenderTarget> Create(std::unique_ptr<Image> &&image);

public:
    RenderTarget(std::vector<std::unique_ptr<Image>> images);

    RenderTarget(Device *device, const Description &description);

    RenderTarget(RenderTarget &&other);

    ~RenderTarget();

    RenderTarget &operator=(RenderTarget &&other);

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return renderPass.get();
        }
        if constexpr (IsPrimitiveOf<Framebuffer, T>())
        {
            return framebuffer.get();
        }
    }

    void SetupFramebuffer()
    {
        std::vector<VkImageView> views;

        for (auto &color : attachments.colors)
        {
            views.emplace_back(color.view->Handle());
        }
        views.emplace_back(attachments.depth.view->Handle());

        framebuffer.reset(new Framebuffer{ device, *renderPass, views, extent });
    }

    void Set(std::shared_ptr<RenderPass> &value)
    {
        renderPass = value;
        SetupFramebuffer();
    }

    uint32_t ColorAttachmentCount()
    {
        return attachments.colors.size();
    }

    const Image &GetColorImage(uint32_t index = 0) const
    {
        return *attachments.colors[index].image;
    }

    void SetupDescriptor();

public:
    virtual uint64_t Descriptor() const override;

private:
    Device *device{ nullptr };

    VkExtent2D extent{};

    std::shared_ptr<RenderPass> renderPass;

    std::unique_ptr<Framebuffer> framebuffer;

    std::unique_ptr<DescriptorSet> descriptorSet;

    std::unique_ptr<ImageDescriptor> descriptor;

    Sampler sampler;

    struct
    {
        Attachment depth;

        std::vector<Attachment> colors;
    } attachments;
};
}
}
