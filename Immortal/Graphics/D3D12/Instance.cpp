#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"

namespace Immortal
{
namespace D3D12
{

Instance::Instance() :
	D3D::Instance{},
	d3d12Library{},
    D3D12CreateDevice{},
    GetDebugInterface{}
{
	LoadSharedObject();

	uint32_t dxgiFactoryFlags = 0;

#if _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		LOG::DEBUG("Enable Direct3D 12 Debug Layer", (void *) (debugController.Get()));
	}
#endif
}

Instance::~Instance()
{
	physicalDevices.clear();
	if (d3d12Library)
	{
		FreeLibrary(d3d12Library);
		d3d12Library = {};
	}
}

SuperDevice *Instance::CreateDevice(int deviceId)
{
	if (deviceId >= physicalDevices.size())
	{
		physicalDevices.resize(deviceId + 1);
	}
	physicalDevices[deviceId] = new PhysicalDevice{this, deviceId};
	return new Device{ physicalDevices[deviceId] };
}

PFN_D3D12SerializeVersionedRootSignature Instance::SerializeVersionedRootSignature;
void Instance::LoadSharedObject()
{
	d3d12Library = LoadLibraryA("d3d12.dll");
	if (!d3d12Library)
	{
		LOG::ERR("Failed to load d3d12.dll");
		return;
	}

	D3D12CreateDevice = (PFN_D3D12CreateDevice) GetProcAddress(d3d12Library, "D3D12CreateDevice");
	if (!D3D12CreateDevice)
	{
		LOG::ERR("Failed to load D3D12CreateDevice");
		return;
	}

#ifdef _DEBUG
	GetDebugInterface = (PFN_D3D12GetDebugInterface)GetProcAddress(d3d12Library, "D3D12GetDebugInterface");
	if (!GetDebugInterface)
	{
		LOG::ERR("Failed to load D3D12GetDebugInterface");
		return;
	}
#endif

	SerializeVersionedRootSignature = (PFN_D3D12SerializeVersionedRootSignature)GetProcAddress(d3d12Library, "D3D12SerializeVersionedRootSignature");
}

}
}
