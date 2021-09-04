#include "impch.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{
RenderContext::RenderContext(Description &desc)
{
    Initialize();
}

RenderContext::RenderContext(const void *handle)
{
    Initialize();
}

void RenderContext::Initialize()
{
#if SLDEBUG
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        LOG::INFO("Enable Debug Layer: {0}", rcast<void*>(debugController.Get()));
    }
#endif
    device = CreateScope<Device>();
}
}
}
