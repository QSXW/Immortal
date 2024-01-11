#pragma once

#include "Shared/IObject.h"
#include "Types.h"

namespace Immortal
{

class Device;
class IMMORTAL_API Sampler : public IObject
{
public:
	virtual ~Sampler() = default;
};

using SuperSampler = Sampler;

}
