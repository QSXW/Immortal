#include "Instance.h"
#include "Device.h"

namespace Immortal
{
namespace OpenGL
{

Instance::Instance()
{

}

Instance::~Instance()
{

}

SuperDevice *Instance::CreateDevice(int deviceId)
{
	return new Device{};
}

}
}
