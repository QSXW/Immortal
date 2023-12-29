#pragma once

#include "Core.h"
#include "Types.h"
#include "Interface/IObject.h"

namespace Immortal
{

class GPUEvent;
class RenderTarget;
class IMMORTAL_API Swapchain : public IObject
{
public:
    using Mode = SwapchainMode;

public:
	virtual ~Swapchain() = default;

    virtual void PrepareNextFrame() = 0;

    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual RenderTarget *GetCurrentRenderTarget() = 0;

protected:
    SwapchainMode mode = SwapchainMode::None;
};

using SuperSwapchain = Swapchain;

}
