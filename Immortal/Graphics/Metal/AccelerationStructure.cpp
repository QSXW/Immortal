#include "AccelerationStructure.h"
#include "Device.h"
#include "Buffer.h"
#include "Metal/MTLAccelerationStructure.hpp"

namespace Immortal
{
namespace Metal
{

AccelerationStructure::AccelerationStructure() :
    Handle<MTL::AccelerationStructure>{}
{

}

AccelerationStructure::~AccelerationStructure()
{

}

}
}
