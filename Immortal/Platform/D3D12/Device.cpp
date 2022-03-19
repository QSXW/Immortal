#include "impch.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Device::Device(ComPtr<IDXGIFactory4> factory) :
    dxgiFactory{ factory.Get() }
{
    if (UseWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        Check(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        Check(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&handle)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        Check(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&handle)));
    }
}

Device::~Device()
{
    IfNotNullThenRelease(handle);
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
