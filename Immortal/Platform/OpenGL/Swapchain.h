#pragma once

#include "Render/LightGraphics.h"
#include "Render/Swapchain.h"
#include "Texture.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace OpenGL
{

class RenderTarget;
class IMMORTAL_API Swapchain : public SuperSwapchain
{
public:
	Swapchain(Window *window, Format format, uint32_t bufferCount, SwapchainMode mode);

	virtual ~Swapchain() override;

    virtual void PrepareNextFrame() override;

	virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperRenderTarget *GetCurrentRenderTarget() override;

public:
	void Present();

	void SetWindowContext(bool trigger);

protected:
	Window *window;

	uint32_t width;

	uint32_t height;

	RenderTarget renderTarget;
};

}
}
