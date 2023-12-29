#pragma once

#include "Render/GPUEvent.h"
#include "Fence.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class IMMORTAL_API GPUEvent : public SuperGPUEvent
{
public:
	GPUEvent(Device *device);

	virtual ~GPUEvent() override;

	virtual void Signal(uint64_t value) override;

	virtual void Wait(uint64_t value, uint64_t timeout) override;

	virtual void Wait(uint64_t timeout) override;

	virtual uint64_t GetCompletionValue() override;

	virtual uint64_t GetSyncPoint() override;

public:
	Fence &GetFence()
	{
		return fence;
	}

	uint64_t GetValue() const
	{
		return value;
	}

	uint64_t GetIncrement()
	{
		return ++value;
	}

protected:
	uint64_t value;

	Fence fence;
};

}
}
