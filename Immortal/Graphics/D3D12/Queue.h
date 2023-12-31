#pragma once

#include "Common.h"
#include "Graphics/Queue.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Swapchain.h"
#include "Graphics/GPUEvent.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Fence;
class CommandList;
class GPUEvent;
class Queue : public SuperQueue
{
public:
    enum class Flag
    {
        None              = D3D12_COMMAND_QUEUE_FLAG_NONE,
        DisableGPUTimeout = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
    };

    using Primitive = ID3D12CommandQueue;
	D3D12_OPERATOR_HANDLE()

public:
	Queue(Device *device, const D3D12_COMMAND_QUEUE_DESC &desc);

    virtual ~Queue() override;

    virtual Anonymous GetBackendHandle() const;

    virtual void WaitIdle(uint32_t timeout) override;

    virtual void Wait(SuperGPUEvent *pEvent) override;

	virtual void Signal(SuperGPUEvent *pEvent) override;

	virtual void Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain *swapchain) override;

	virtual void Present(SuperSwapchain *swapchain, SuperGPUEvent **ppSignalEvent, uint32_t eventCount) override;

    void ExecuteCommandLists(CommandList *pCommandList);

    HRESULT Signal(Fence *pFence, uint64_t value);

    void ExecuteCommandLists(ID3D12CommandList **ppCommandList, uint32_t num = 1)
    {
        handle->ExecuteCommandLists(num, ppCommandList);
    }

    HRESULT Signal(ID3D12Fence *fence, UINT64 value)
    {
        return handle->Signal(fence, value);
    }

protected:
	URef<GPUEvent> gpuEvent;
};

}
}
