#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/GPUEvent.h"
#include "Metal/MTLEvent.hpp"

namespace Immortal
{
namespace Metal
{

class Device;
class IMMORTAL_API GPUEvent : public SuperGPUEvent, public Handle<MTL::Event>
{
public:
	METAL_SWAPPABLE(GPUEvent)

public:
	GPUEvent();

	GPUEvent(Device *device);

	virtual ~GPUEvent() override;

	virtual void Signal(uint64_t value) override;

	virtual void Wait(uint64_t value, uint64_t timeout) override;

	virtual void Wait(uint64_t timeout) override;

	virtual uint64_t GetCompletionValue() override;

	virtual uint64_t GetSyncPoint() override;

public:
	void Signal(MTL::CommandBuffer *commandBuffer, uint64_t value);

	void Wait(MTL::CommandBuffer *commandBuffer, uint64_t value);

public:
	uint64_t GetValue() const
	{
		return value;
	}

	uint64_t GetIncrement()
	{
		return ++value;
	}

	void Swap(GPUEvent &other)
	{
		Handle::Swap(other);
		std::swap(value,  other.value );
	}

protected:
	Device *device;

	uint64_t value;
};

}
}
