#pragma once

#include "Interface/IObject.h"
#include "Types.h"
#include "Framework/Window.h"

namespace Immortal
{

class Device;
class IMMORTAL_API Instance : public IObject
{
public:
	virtual ~Instance() = default;

	virtual Device *CreateDevice(int deviceId) = 0;

public:
	static Instance *CreateInstance(BackendAPI apiType, Window::Type windowType);
};

using SuperInstance = Instance;

}
