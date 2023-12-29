#pragma once

#include "Render/Instance.h"
#include "Render/Device.h"
#include "Platform/D3D/Interface.h"
#include "Common.h"

namespace Immortal
{
namespace D3D12
{

using PFN_D3D12CreateDevice      = decltype(::D3D12CreateDevice)*;
using PFN_D3D12GetDebugInterface = decltype(::D3D12GetDebugInterface)*;
using PFN_D3D12SerializeVersionedRootSignature = decltype(D3D12SerializeVersionedRootSignature) *;

class IMMORTAL_API Instance : public D3D::Instance
{
public:
	Instance();

	virtual ~Instance() override;

	virtual SuperDevice *CreateDevice(int deviceId) override;

protected:
	void LoadSharedObject();

public:
	PFN_D3D12CreateDevice  D3D12CreateDevice;

	PFN_D3D12GetDebugInterface GetDebugInterface;
	
	static PFN_D3D12SerializeVersionedRootSignature SerializeVersionedRootSignature;

protected:
	HMODULE d3d12Library;

	std::vector<URef<PhysicalDevice>> physicalDevices;
};

}
}
