#pragma once

#include "Common.h"

namespace Immortal
{

namespace D3D11
{

class Device;
class Swapchain
{
public:
    using Primitive = IDXGISwapChain;
    D3D_OPERATOR_HANDLE()

public:
    Swapchain();

    ~Swapchain();

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

private:
    uint32_t bufferIndex = 0;
};

}
}
