#pragma once

#include "Render/Instance.h"

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
