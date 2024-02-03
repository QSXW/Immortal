#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderTarget.h"
#include "Metal/MTLDrawable.hpp"
#include "QuartzCore/CAMetalDrawable.hpp"
#include "RenderTarget.h"
#include <QuartzCore/CAMetalLayer.hpp>

namespace Immortal
{

class Window;
namespace Metal
{

class GPUEvent;
class Queue;
class Device;
class RenderTarget;
class IMMORTAL_API Swapchain : public SuperSwapchain, public Handle<CA::MetalLayer>
{
public:
    METAL_SWAPPABLE(Swapchain)

public:
    Swapchain();

    Swapchain(Device *device, Queue *queue, Window *window, Format format,  uint32_t bufferCount, SwapchainMode mode = SwapchainMode::None);

    virtual ~Swapchain() override;

    virtual void PrepareNextFrame() override;

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual SuperRenderTarget *GetCurrentRenderTarget() override;

public:
    void Present(Queue *queue);

public:
    MTL::Drawable *GetDrawable() const
    {
        return drawable;
    }

    void Swap(Swapchain &other)
    {
        Handle::Swap(other);
        std::swap(renderTarget, other.renderTarget);
    }

protected:
    Window *window;

    CA::MetalDrawable *drawable;

    URef<RenderTarget> renderTarget;
};

}
}
