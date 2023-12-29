#include "PhysicalDevice.h"
#include "Instance.h"

namespace Immortal
{
namespace D3D
{

PhysicalDevice::PhysicalDevice() :
    handle{},
    instance{},
    desc{}
{

}

PhysicalDevice::PhysicalDevice(Instance *instance, int deviceId) :
    instance{ instance },
    desc{}
{
	if (deviceId == AUTO_DEVICE_ID)
	{
		deviceId = 0;
	}

	instance->EnumerateAdapter(deviceId, &handle);

    Check(handle->GetDesc(&desc));
}

PhysicalDevice::~PhysicalDevice()
{
	handle.Reset();
}

DXGI_ADAPTER_DESC PhysicalDevice::GetAdapterDesc()
{
    DXGI_ADAPTER_DESC desc{};
	Check(handle->GetDesc(&desc));

    return desc;
}

}
}
