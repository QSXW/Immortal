#pragma once

#include "Graphics/Instance.h"
#include "Interface.h"

namespace Immortal
{
namespace D3D
{

using PFN_CreateDXGIFactory2 = decltype(::CreateDXGIFactory2)*;

class PhysicalDevice;
class IMMORTAL_API Instance : public SuperInstance
{
public:
	using Primitive = IDXGIFactory4;
	D3D_OPERATOR_HANDLE()

public:
	Instance();

	virtual ~Instance();

public:
	void EnumerateAdapter(uint32_t deviceId, IDXGIAdapter1 **ppAdapter);

public:
	void Swap(Instance &other)
	{
		std::swap(handle, other.handle);
	}

	PFN_CreateDXGIFactory2 CreateDXGIFactory2;

protected:
	HMODULE dxgiLibrary;

	std::vector<URef<PhysicalDevice>> physicalDevices;
};

}
}
