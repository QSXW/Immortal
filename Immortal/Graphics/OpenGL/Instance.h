#pragma once

#include "Graphics/Instance.h"
#include "Graphics/Device.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Instance : public SuperInstance
{
public:
	Instance();

	virtual ~Instance() override;

	virtual SuperDevice *CreateDevice(int deviceId) override;
};

}
}
