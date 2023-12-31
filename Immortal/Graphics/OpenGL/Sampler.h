#pragma once

#include "Graphics/Sampler.h"
#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Sampler : public SuperSampler, public Handle
{
public:
	Sampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod);

    virtual ~Sampler() override;
};

}
}
