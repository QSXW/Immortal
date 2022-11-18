#include "Device.h"
#include "Swapchain.h"

namespace Immortal
{
namespace D3D11
{

Device::Device(int deviceId) :
    featureLevel{}
{
	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	Check(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)), "Failed to create DXGI Factory");

	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pDeviceContext;

	if (deviceId == AUTO_DEVICE_ID)
	{
		GetHardwareAdapter(dxgiFactory.Get(), &adapter);
	}
	else
	{
		dxgiFactory->EnumAdapters1(deviceId, &adapter);
	}

	Check(D3D11CreateDevice(
	    adapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
	    0,
	    deviceFlags,
	    FeatureLevels,
	    SL_ARRAY_LENGTH(FeatureLevels),
	    D3D11_SDK_VERSION,
	    &pDevice,
	    &featureLevel,
	    &pDeviceContext
	));

	pDevice.As<ID3D11Device5>(&handle);
	pDeviceContext.As<ID3D11DeviceContext4>(&context);
}

Device::~Device()
{

}

HRESULT Device::CreateSwapchain(Swapchain *pSwapchain, DXGI_SWAP_CHAIN_DESC *pDesc)
{
	return dxgiFactory->CreateSwapChain(handle.Get(), pDesc, &(*pSwapchain));
}

}
}
