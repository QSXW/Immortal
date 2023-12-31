#pragma once

#include "Graphics/Queue.h"
#include "Graphics/LightGraphics.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Queue : public SuperQueue
{
public:
	virtual ~Queue() override;

	virtual void WaitIdle(uint32_t timeout) override;

	virtual void Wait(SuperGPUEvent *pEvent) override;

	virtual void Signal(SuperGPUEvent *pEvent) override;

	virtual void Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain *swapchain = nullptr) override;

	virtual void Present(SuperSwapchain *swapchain, SuperGPUEvent **ppSignalEvents, uint32_t eventCount) override;
};

}
}