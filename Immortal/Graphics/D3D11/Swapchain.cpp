#include "Swapchain.h"
#include "Device.h"
#include "Window.h"
#include "RenderTarget.h"
#include "Texture.h"

namespace Immortal
{
namespace D3D11
{

Swapchain::Swapchain(Device *device) :
    NonDispatchableHandle{ device },
    handle{},
    window{},
    mode{}
{

}

Swapchain::Swapchain(Device *device, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) :
    NonDispatchableHandle{device},
    handle{},
    window{ window },
    mode{ mode }
{
    uint32_t width  = window->GetWidth();
    uint32_t height = window->GetHeight();

    DXGI_SWAP_CHAIN_DESC desc = {
        .BufferDesc   = {
            .Width            = width,
            .Height           = height,
            .RefreshRate      = { .Numerator = 0, .Denominator = 1 },
            .Format           = format,
            .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
            .Scaling          = DXGI_MODE_SCALING_UNSPECIFIED,
        },
        .SampleDesc   = { .Count = 1, .Quality = 0 },
        .BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount  = bufferCount,
        .OutputWindow = (HWND)window->GetBackendHandle(),
        .Windowed     = true,
        .SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
    };

    auto dxgiFactory = device->GetDXGIFactory();
    Check(dxgiFactory->CreateSwapChain((ID3D11Device5 *) *device, &desc, &handle));

    CreateRenderTarget();
}

Swapchain::~Swapchain()
{
    handle.Reset();
}

void Swapchain::PrepareNextFrame()
{

}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
	renderTarget->ClearAttachments();

    DXGI_SWAP_CHAIN_DESC desc{};
    Check(handle->GetDesc(&desc));
    Check(handle->ResizeBuffers(
        desc.BufferCount,
        width,
        height,
        DXGI_FORMAT_UNKNOWN,
        desc.Flags
    ));

    CreateRenderTarget();
}

SuperRenderTarget *Swapchain::GetCurrentRenderTarget()
{
    return renderTarget;
}

void Swapchain::Present()
{
    handle->Present((uint32_t)mode, 0);
}

void Swapchain::CreateRenderTarget()
{
	DXGI_SWAP_CHAIN_DESC desc{};
	Check(handle->GetDesc(&desc));

    if (!renderTarget)
    {
		renderTarget = new RenderTarget{};
    }

	ComPtr<ID3D11Texture2D> pBackBuffer;
	Check(handle->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));

	Texture texture{ device, pBackBuffer };
	renderTarget->AddColorAttachment(std::move(texture));
}

}
}
