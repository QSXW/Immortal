#pragma once

#include "Interface.h"
#include "Render/PhysicalDevice.h"

namespace Immortal
{
namespace D3D
{

class Instance;
class IMMORTAL_API PhysicalDevice : public SuperPhysicalDevice
{
public:
    using Primitive = IDXGIAdapter1;
	D3D_OPERATOR_HANDLE()

public:
	PhysicalDevice();

    PhysicalDevice(Instance *instance, int deviceId);

    virtual ~PhysicalDevice() override;

public:
	DXGI_ADAPTER_DESC GetAdapterDesc();

public:
    Instance *GetInstance() const
    {
		return instance;
    }

    void Swap(PhysicalDevice &other)
    {
		std::swap(handle,   other.handle  );
		std::swap(instance, other.instance);
    }

protected:
	Instance *instance;

    DXGI_ADAPTER_DESC desc;
};

}
}
