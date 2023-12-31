#pragma once

#include "Interface/IObject.h"
#include "Types.h"
namespace Immortal
{

class IMMORTAL_API PipelineCache : public IObject
{
public:
	virtual ~PipelineCache() = default;

    virtual void GetData(void **pData, size_t *size) = 0;
};

using SuperPipelineCache = PipelineCache;

}
