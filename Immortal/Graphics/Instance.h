#pragma once

#include "Shared/IObject.h"
#include "Types.h"
#include "Window.h"

namespace Immortal
{

class Device;
class PhysicalDevice;
class IMMORTAL_API Instance : public IObject
{
public:
	virtual ~Instance() = default;

	virtual Device *CreateDevice(int deviceId) = 0;

	virtual int EnumeratePhysicalDevice(uint32_t *numPhysicalDevice, PhysicalDevice **ppPhysicalDevice)
	{
		*numPhysicalDevice = 0;
		return 0;
	}

public:
	static Instance *CreateInstance(BackendAPI apiType, Window::Type windowType);
};

using SuperInstance = Instance;

}
