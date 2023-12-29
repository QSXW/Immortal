#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"

namespace Immortal
{
namespace D3D11
{

Instance::Instance() :
    D3D::Instance{},
    d3d11Library{}
{
	d3d11Library = LoadLibraryA("d3d11.dll");
	if (!d3d11Library)
	{
		LOG::ERR("Failed to load d3d11.dll");
		return;
	}

	D3D11CreateDevice = (PFN_D3D11CreateDevice) GetProcAddress(d3d11Library, "D3D11CreateDevice");
	if (!D3D11CreateDevice)
	{
		LOG::ERR("Failed to load D3D11CreateDevice");
		return;
	}
}

Instance::~Instance()
{
	if (d3d11Library)
	{
		FreeLibrary(d3d11Library);
		d3d11Library = {};
	}
}

SuperDevice *Instance::CreateDevice(int deviceId)
{
	if (deviceId >= physicalDevices.size())
	{
		physicalDevices.resize(deviceId + 1);
	}
	physicalDevices[deviceId] = new PhysicalDevice{ this, deviceId };
	return new Device{ physicalDevices[deviceId] };
}

}
}
