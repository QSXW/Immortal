#include "Device.h"
#include "Framework/DLLLoader.h"
#include <dxgidebug.h>

namespace Immortal
{
namespace D3D12
{

Device::Device()
{
    uint32_t dxgiFactoryFlags = 0;

#if _DEBUG
    ComPtr<ID3D12Debug> debugController;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        LOG::INFO("Enable Debug Layer: {0}", rcast<void*>(debugController.Get()));
    }
#endif
    Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)), "Failed to create DXGI Factory");

    if (UseWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        Check(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        Check(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&handle)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(dxgiFactory.Get(), &hardwareAdapter);
        Check(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&handle)));
    }
}

Device::~Device()
{
    handle.Reset();
    dxgiFactory.Reset();

#ifdef _DEBUG
    DLLLoader loader{ "dxgidebug.dll" };
    if (loader.IsAvailable())
    {
        ComPtr<IDXGIDebug> dxgiDebug;
        auto __DXGIGetDebugInterface = loader.GetFunc<decltype(&DXGIGetDebugInterface)>("DXGIGetDebugInterface");
        Check(__DXGIGetDebugInterface(IID_PPV_ARGS(&dxgiDebug)));
        Check(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
    }
#endif
}

DXGI_ADAPTER_DESC Device::GetAdapterDesc()
{
    ComPtr<IDXGIAdapter> dxgiAdapter{ nullptr };
    Check(dxgiFactory->EnumAdapters(0, &dxgiAdapter));

    DXGI_ADAPTER_DESC adapterDesc{};
    Check(dxgiAdapter->GetDesc(&adapterDesc));

    return adapterDesc;
}

}
}
