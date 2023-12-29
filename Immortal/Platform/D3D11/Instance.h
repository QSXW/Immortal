#pragma once

#include "Render/Instance.h"
#include "Render/Device.h"

namespace Immortal
{
namespace D3D11
{

using PFN_D3D11CreateDevice = decltype(::D3D11CreateDevice)*;

class IMMORTAL_API Instance : public D3D::Instance
{
public:
	Instance();

	virtual ~Instance() override;

	virtual SuperDevice *CreateDevice(int deviceId) override;

public:
	PFN_D3D11CreateDevice D3D11CreateDevice;

protected:
	HMODULE d3d11Library;
};

}
}
