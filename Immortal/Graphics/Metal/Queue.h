#pragma once

#include "Common.h"
#include "Handle.h"
#include "GPUEvent.h"
#include "Graphics/Queue.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Swapchain.h"
#include "Graphics/GPUEvent.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace Metal
{

class Device;
class GPUEvent;
class CommandBuffer;
class IMMORTAL_API Queue : public SuperQueue, public Handle<MTL::CommandQueue>
{
public:
    METAL_SWAPPABLE(Queue)

public:
    Queue();

	Queue(Device *device, MTL::CommandQueue *queue);

    virtual ~Queue() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual void WaitIdle(uint32_t timeout) override;

    virtual void Wait(SuperGPUEvent *pEvent) override;

	virtual void Signal(SuperGPUEvent *pEvent) override;

	virtual void Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain *swapchain) override;

	virtual void Present(SuperSwapchain *swapchain, SuperGPUEvent **ppSignalEvent, uint32_t eventCount) override;

public:
    void Swap(Queue &other)
    {
        Handle::Swap(other);
        std::swap(gpuEvent, gpuEvent);
    }

protected:
    URef<GPUEvent> gpuEvent;
};

}
}
