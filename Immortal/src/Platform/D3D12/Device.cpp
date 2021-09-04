#include "impch.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Device::Device()
{
    Check(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "Failed to create DXGI Factory");

    if (UseWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        Check(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        Check(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&handle)));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        Check(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&handle)));
    }
}

}
}
