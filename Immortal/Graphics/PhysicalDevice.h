#pragma once

#include "Shared/IObject.h"
#include "Types.h"

namespace Immortal
{

struct PhysicalDeviceDescription
{
	wchar_t description[128];
	uint32_t venderId;
	uint32_t deviceId;
};

class IMMORTAL_API PhysicalDevice : public IObject
{
public:
    using Type = PhysicalDeviceType;

	virtual ~PhysicalDevice() = default;

	virtual PhysicalDeviceDescription GetDescription()
	{
		return {};
	}
};

using SuperPhysicalDevice = PhysicalDevice;

}
