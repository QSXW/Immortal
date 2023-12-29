#pragma once

#include "Render/RenderTarget.h"

#include "Common.h"
#include "Device.h"
#include "Attachment.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Submitter.h"
#include "Algorithm/LightArray.h"
#include "Texture.h"

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

class IMMORTAL_API RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;
    VKCPP_SWAPPABLE(RenderTarget)

public:
    RenderTarget();

    RenderTarget(Device *device);

    ~RenderTarget();

    virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperTexture *GetColorAttachment(uint32_t index) override;

	virtual SuperTexture *GetDepthAttachment() override;

public:
    void Release();

    void SetColorAttachment(uint32_t index, Texture &&texture);

    void Construct(std::vector<Texture> &colorAttachments, Texture &&depthAttachment, bool isPresented = false);

    void Invalidate();

public:
    template <class T>
    requires std::is_same_v<T, RenderPass> || std::is_same_v<T, Framebuffer>
    T *Get()
    {
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return &renderPass;
        }
        if constexpr (IsPrimitiveOf<Framebuffer, T>())
        {
            return &framebuffer;
        }
    }

    const RenderPass &GetRenderPass() const
    {
        return renderPass;
    }

    const Framebuffer &GetFramebuffer()
    {
        return framebuffer;
    }

    const std::vector<Texture> &GetColorAttachments() const
    {
		return attachments.colors;
    }

    const Texture &InternalGetDepthAttachment() const
    {
		return attachments.depth;
    }

    uint32_t ColorAttachmentCount() const
    {
        return attachments.colors.size();
    }

    void SetSwapchainTarget()
    {
		isSwapchainTarget = true;
    }

    bool IsSwapchainTarget() const
    {
		return isSwapchainTarget;
    }

    void Swap(RenderTarget &other)
    {
        std::swap(device,      other.device     );
        std::swap(renderPass,  other.renderPass );
        std::swap(framebuffer, other.framebuffer);
    }

protected:
    void Create();

private:
    Device *device{ nullptr };

    RenderPass renderPass;

    Framebuffer framebuffer;

    struct
    {
        Texture depth;

        std::vector<Texture> colors;
    } attachments;

    bool isSwapchainTarget = false;
};

}
}
