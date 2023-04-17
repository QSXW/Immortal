#include "Device.h"
#include "Framework/DLLLoader.h"
#include "RenderContext.h"
#include <dxgidebug.h>

namespace Immortal
{
namespace D3D12
{

Device::Device(int deviceId)
{
    uint32_t dxgiFactoryFlags = 0;

#if _DEBUG
    ComPtr<ID3D12Debug> debugController;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        LOG::INFO("Enable Debug Layer: {0}", (void *)(debugController.Get()));
    }
#endif
    Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)), "Failed to create DXGI Factory");

    if (deviceId == AUTO_DEVICE_ID)
    {
        GetHardwareAdapter(dxgiFactory.Get(), &adapter);
    }
    else
    {
        dxgiFactory->EnumAdapters1(deviceId, &adapter);
    }

    Check(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&handle)));
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

void Device::EnumerateAdapters(std::vector<AdapterProperty> &pAdapters)
{
    for (UINT adapterIndex = 0; ; adapterIndex++)
    {
        ComPtr<IDXGIAdapter1> pAdapter;
        if (dxgiFactory->EnumAdapters1(adapterIndex, pAdapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        AdapterProperty props = { .pAdapter = pAdapter };
        Check(pAdapter->GetDesc(&props.desc));
        pAdapters.emplace_back(props);
    }
}

DXGI_ADAPTER_DESC Device::GetAdapterDesc()
{
    DXGI_ADAPTER_DESC adapterDesc{};
    Check(adapter->GetDesc(&adapterDesc));

    return adapterDesc;
}

}
}
