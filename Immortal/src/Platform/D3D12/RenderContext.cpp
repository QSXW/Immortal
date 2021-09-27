#include "impch.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{
RenderContext::RenderContext(Description &descrition) :
    desc{ descrition }
{
    INIT();
}

RenderContext::RenderContext(const void *handle)
{
    INIT();
}

RenderContext::~RenderContext()
{
    IfNotNullThen<FreeLibrary>(hmodule);
}

void RenderContext::INIT()
{
#if SLDEBUG
    hmodule = LoadLibrary(L"D3d12SDKLayers.dll");
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        LOG::INFO("Enable Debug Layer: {0}", rcast<void*>(debugController.Get()));
    }
#endif

    Check(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI Factory");
    device = std::make_unique<Device>(factory);

    {
        Queue::Description desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

        queue = device->CreateQueue(desc);
    }

    {
        Swapchain::Description desc{};
        CleanObject(&desc);
        desc.BufferCount       = 3;
        desc.BufferDesc.Width  = 0;
        desc.BufferDesc.Height = 0;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.OutputWindow      = rcast<HWND>(this->desc.WindowHandle);
        desc.SampleDesc.Count  = 1;
        desc.Windowed          = TRUE;

        swapchain = std::make_unique<Swapchain>(factory, queue->Handle(), desc);
    }
}

}
}
