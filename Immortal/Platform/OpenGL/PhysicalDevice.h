#pragma once

#include "Render/PhysicalDevice.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API PhysicalDevice : public SuperPhysicalDevice
{
public:
	PhysicalDevice(int deviceId);

	virtual ~PhysicalDevice() override;
};

}
}
