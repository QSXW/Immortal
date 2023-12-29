#pragma once

#include "Render/GPUEvent.h"
#include "Semaphore.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class IMMORTAL_API GPUEvent : public SuperGPUEvent, public TimelineSemaphore
{
public:
	VKCPP_SWAPPABLE(GPUEvent)

public:
    GPUEvent(Device *device = nullptr);

    GPUEvent(Semaphore semaphore);

    virtual ~GPUEvent() override;

    virtual void Signal(uint64_t value) override;

    virtual void Wait(uint64_t value, uint64_t timeout) override;

    virtual void Wait(uint64_t timeout) override;

    virtual uint64_t GetCompletionValue() override;

    virtual uint64_t GetSyncPoint() override;

public:
    uint64_t GetValue() const
    {
		return value;
    }

    uint64_t GetIncrement()
    {
		return ++value;
    }

    bool IsTimeline() const
    {
		return device != nullptr;
    }

    VkSemaphore GetSemaphore() const
    {
		return handle;
    }

    void Swap(GPUEvent &other)
    {
		TimelineSemaphore::Swap(other);
		std::swap(device, other.device);
    }

protected:
	Device *device;
};

}
}
