#pragma once

#include "Common.h"
#include "Graphics/PhysicalDevice.h"

namespace Immortal
{
namespace Metal
{

class PhysicalDevice : public SuperPhysicalDevice
{
public:
    PhysicalDevice(int deviceId);
};

}
}
