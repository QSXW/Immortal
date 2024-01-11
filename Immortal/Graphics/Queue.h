#pragma once

#include "Core.h"
#include "Device.h"
#include "Shared/IObject.h"

namespace Immortal
{

class CommandBuffer;
class Swapchain;
class GPUEvent;
class Swapchain;

/**
 * Queue is known as a command queue in Direct3D 12 and Vulkan
 * for submitting execution data of the command list/buffer.
 *
 */
class IMMORTAL_API Queue : public IObject
{
public:
    using Type = QueueType;

    using Priority = QueuePriority;

public:
	virtual ~Queue() = default;

    /**
     * @brief Get the backend handle
     * Vulkan: VkQueue
     * D3D12:  A pointer to the ID3D12CommandQueue
     */
    virtual Anonymous GetBackendHandle() const { return nullptr; }

    /**
	 * @brief Wait for the queue to be idle
	 */
    virtual void WaitIdle(uint32_t timeout = 0xffffffff) = 0;

    /**
	 * @brief Wait for a GPU event before executing any other GPU commands
	 */
    virtual void Wait(GPUEvent *pEvent) = 0;

    /**
     * @brief Signal a GPU event
     */
    virtual void Signal(GPUEvent *pEvent) = 0;

    /**
	 * @brief Submit a swapchain to queue for present
	 * @param swapchain     The specific swapchain
	 * @param pSignalEvents The event to be signaled after the present
	 * @param eventCount    The count of the specific signal events
     * @param swapchain     Workaround for Vulkan. Can be ignored if the Vulkan backend is not used.
	 */
	virtual void Submit(CommandBuffer **ppCommandBuffer, size_t count, GPUEvent **ppSignalEvents = nullptr, uint32_t eventCount = 0, Swapchain *swapchain = nullptr) = 0;

    /**
     * @brief Submit a swapchain to queue for present
     * @param swapchain     The specific swapchain
     * @param pSignalEvents The event to be signaled after the present
     * @param eventCount    The count of the specific signal events
     */
	virtual void Present(Swapchain *swapchain, GPUEvent **ppSignalEvent = nullptr, uint32_t eventCount = 0) = 0;

public:
	void Submit(CommandBuffer *pCommandBuffer, GPUEvent *pSignalEvent = nullptr, Swapchain *swapchain = nullptr);
};

using SuperQueue = Queue;

}
