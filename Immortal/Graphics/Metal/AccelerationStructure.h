#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/AccelerationStructure.h"
#include "Metal/MTLAccelerationStructure.hpp"

namespace Immortal
{
namespace Metal
{

class Buffer;
class Device;
class IMMORTAL_API AccelerationStructure : public SuperAccelerationStructure, public Handle<MTL::AccelerationStructure>
{
public:
	METAL_SWAPPABLE(AccelerationStructure)

public:
	AccelerationStructure();

	~AccelerationStructure();

public:
	void Swap(AccelerationStructure &other)
	{
		Handle::Swap(other);
	}
};

}
}
