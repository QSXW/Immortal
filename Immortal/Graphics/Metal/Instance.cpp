#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"

namespace Immortal
{
namespace Metal
{

Instance::Instance() :
	physicalDevices{}
{
}

Instance::~Instance()
{
	physicalDevices.clear();
}

SuperDevice *Instance::CreateDevice(int deviceId)
{
	physicalDevices.resize(deviceId + 1);
	return new Device{ physicalDevices[deviceId] };
}

}
}
