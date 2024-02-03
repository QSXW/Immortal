#pragma once

#include "Graphics/Instance.h"
#include "Graphics/Device.h"
#include "Common.h"

namespace Immortal
{
namespace Metal
{

class PhysicalDevice;
class IMMORTAL_API Instance : public SuperInstance
{
public:
	Instance();

	virtual ~Instance() override;

	virtual SuperDevice *CreateDevice(int deviceId) override;

protected:
	std::vector<URef<PhysicalDevice>> physicalDevices;
};

}
}
