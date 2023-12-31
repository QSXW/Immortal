#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Swapchain.h"

namespace Immortal
{

namespace D3D11
{

class Device;
class RenderTarget;
class IMMORTAL_API Swapchain : public SuperSwapchain, public NonDispatchableHandle
{
public:
    using Primitive = IDXGISwapChain;
    D3D_OPERATOR_HANDLE()
    D3D11_SWAPPABLE(Swapchain)

public:
	Swapchain(Device *device = nullptr);

	Swapchain(Device *device, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode);

    virtual ~Swapchain() override;

	virtual void PrepareNextFrame() override;

	virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperRenderTarget *GetCurrentRenderTarget() override;

public:
	void Present();

    void CreateRenderTarget();

public:
    HRESULT GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
    {
		return handle->GetDesc(pDesc);
    }

    HRESULT ResizeBuffers(UINT width, UINT height, DXGI_FORMAT newFormat, UINT flags, UINT bufferCount = 0)
    {
      return handle->ResizeBuffers(
          bufferCount,
          width,
          height,
          newFormat,
          flags);
    }

    HRESULT GetBuffer(UINT Buffer, ID3D11Texture2D **ppTexture2D)
    {
		return handle->GetBuffer(Buffer, IID_PPV_ARGS(ppTexture2D));
    }

    HRESULT Present(UINT syncInterval, UINT flags)
    {
		return handle->Present(syncInterval, flags);
    }

    void Swap(Swapchain &other)
    {
		NonDispatchableHandle::Swap(other);
		std::swap(handle,       other.handle      );
		std::swap(window,       other.window      );
		std::swap(renderTarget, other.renderTarget);
		std::swap(mode,         other.mode        );
    }

protected:
	Window *window;

    URef<RenderTarget> renderTarget;

	SwapchainMode mode;
};

}
}
