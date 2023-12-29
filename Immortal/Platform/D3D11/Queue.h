#pragma once

#include "Render/Queue.h"
#include "Render/LightGraphics.h"

namespace Immortal
{
namespace D3D11
{

class IMMORTAL_API Queue : public SuperQueue
{
public:
	Queue();

    virtual ~Queue() override;

    virtual void WaitIdle(uint32_t timeout) override;

    virtual void Wait(SuperGPUEvent *pEvent) override;

    virtual void Signal(SuperGPUEvent *pEvent) override;

    virtual void Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain *swapchain = nullptr) override;

    virtual void Present(SuperSwapchain *swapchain, SuperGPUEvent **ppSignalEvents, uint32_t eventCount) override;
};

}
}
