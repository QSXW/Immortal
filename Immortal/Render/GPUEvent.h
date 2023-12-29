#pragma once

#include "Core.h"
#include "Device.h"
#include "Interface/IObject.h"

namespace Immortal
{

class IMMORTAL_API GPUEvent : public IObject
{
public:
	virtual ~GPUEvent() = default;

    virtual void Signal(uint64_t value) = 0;

    virtual void Wait(uint64_t value, uint64_t timeout) = 0;

    virtual void Wait(uint64_t timeout) = 0;

    virtual uint64_t GetCompletionValue() = 0;

    virtual uint64_t GetSyncPoint() = 0;
};

using SuperGPUEvent = GPUEvent;

}
