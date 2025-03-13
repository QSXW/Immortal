#pragma once

#include "Graphics/Instance.h"
#include "Graphics/Device.h"
#include "Common.h"
#include "Config.h"

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
#if HAVE_AGILITY_SDK
	HMODULE d3d12CoreLibrary;
#endif

	HMODULE d3d12Library;
};

}
}
