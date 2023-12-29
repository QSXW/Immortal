#pragma once

#include "Interface/IObject.h"
#include "Types.h"

namespace Immortal
{

class IMMORTAL_API PhysicalDevice : public IObject
{
public:
    using Type = PhysicalDeviceType;

	virtual ~PhysicalDevice() = default;
};

using SuperPhysicalDevice = PhysicalDevice;

}
