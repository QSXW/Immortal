#pragma once

#include "Common.h"
#include "Semaphore.h"
#include "Handle.h"
#include "Algorithm/LightArray.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Queue.h"
#include "Graphics/GPUEvent.h"
#include "Graphics/Swapchain.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class DeviceQueue : public Handle<VkQueue>
{
public:
	VKCPP_SWAPPABLE(DeviceQueue)

public:
    VkResult Submit(uint32_t submitCount, VkSubmitInfo const *pSubmits, VkFence fence)
    {
        return vkQueueSubmit(handle, submitCount, pSubmits, fence);
    }

    VkResult WaitIdle()
    {
        return vkQueueWaitIdle(handle);
    }

    VkResult BindSparse(uint32_t bindInfoCount, VkBindSparseInfo const *pBindInfo, VkFence fence)
    {
        return vkQueueBindSparse(handle, bindInfoCount, pBindInfo, fence);
    }

    VkResult PresentKHR(VkPresentInfoKHR const *pPresentInfo)
    {
        return vkQueuePresentKHR(handle, pPresentInfo);
    }

    void BeginDebugUtilsLabelEXT(VkDebugUtilsLabelEXT const *pLabelInfo)
    {
        vkQueueBeginDebugUtilsLabelEXT(handle, pLabelInfo);
    }

    void EndDebugUtilsLabelEXT()
    {
        vkQueueEndDebugUtilsLabelEXT(handle);
    }

    void InsertDebugUtilsLabelEXT(VkDebugUtilsLabelEXT const *pLabelInfo)
    {
        vkQueueInsertDebugUtilsLabelEXT(handle, pLabelInfo);
    }

    void GetCheckpointDataNV(uint32_t *pCheckpointDataCount, VkCheckpointDataNV *pCheckpointData)
    {
        vkGetQueueCheckpointDataNV(handle, pCheckpointDataCount, pCheckpointData);
    }

    VkResult SetPerformanceConfigurationINTEL(VkPerformanceConfigurationINTEL configuration)
    {
        return vkQueueSetPerformanceConfigurationINTEL(handle, configuration);
    }

    VkResult Submit2(uint32_t submitCount, VkSubmitInfo2 const *pSubmits, VkFence fence)
    {
        return vkQueueSubmit2(handle, submitCount, pSubmits, fence);
    }

    void GetCheckpointData2NV(uint32_t *pCheckpointDataCount, VkCheckpointData2NV *pCheckpointData)
    {
        vkGetQueueCheckpointData2NV(handle, pCheckpointDataCount, pCheckpointData);
    }

public:
	DeviceQueue(Device *device = nullptr);

    DeviceQueue(Device *device, uint32_t familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, uint32_t index);

    virtual ~DeviceQueue();

public:
    VkResult Submit(const VkSubmitInfo &submitInfo, VkFence fence)
    {
        return Submit(1, &submitInfo, fence);
    }

    VkQueueFamilyProperties &Properties()
    {
        return properties;
    }

    VkBool32 &IsPresentSupported()
    {
        return presented;
    }

    VkResult Present(const VkPresentInfoKHR *pPresentInfo)
    {
        if (!presented)
        {
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
        }

        return PresentKHR(pPresentInfo);
    }

    uint32_t GetFamilyIndex() const
    {
		return familyIndex;
    }

    Device *GetDevice() const
    {
		return device;
    }

    void Occupy()
    {
		used = true;
    }

    void DeOccupy()
    {
		used = false;
    }

    const std::atomic<bool> &IsOccupied() const
    {
		return used;
    }

    void Swap(DeviceQueue &other)
    {
		Handle::Swap(other);
		std::swap(device,      other.device     );
		std::swap(familyIndex, other.familyIndex);
		std::swap(index,       other.index      );
		std::swap(presented,   other.presented  );
        std::swap(properties,  other.properties );
    }

protected:
    Device *device;

    uint32_t familyIndex;

    uint32_t index;

    VkBool32 presented;

    VkQueueFamilyProperties properties{};

    std::atomic<bool> used;
};

class IMMORTAL_API Queue : public SuperQueue
{
public:
    using Super = SuperQueue;
	VKCPP_SWAPPABLE(Queue)

public:
	Queue(DeviceQueue *queue = nullptr);

    virtual ~Queue() override;

	virtual void WaitIdle(uint32_t timeout) override;

	virtual void Wait(SuperGPUEvent *pEvent) override;

	virtual void Signal(SuperGPUEvent *pEvent) override;

	virtual void Submit(SuperCommandBuffer **ppCommandBuffer, size_t count, SuperGPUEvent **ppSignalEvents, uint32_t eventCount, SuperSwapchain *swapchain) override;

	virtual void Present(SuperSwapchain *swapchain, SuperGPUEvent **ppSignalEvent, uint32_t eventCount) override;

public:
	uint32_t GetFamilyIndex() const
	{
		return queue->GetFamilyIndex();
	}

	void Swap(Queue &other)
    {
		std::swap(queue,                       other.queue                      );
		std::swap(executionCompleteSemaphores, other.executionCompleteSemaphores);
		std::swap(waitSemaphores,              other.waitSemaphores             );
		std::swap(waitPipelineStageFlags,      other.waitPipelineStageFlags     );
    }

protected:
	DeviceQueue *queue;

    std::vector<Semaphore> executionCompleteSemaphores;

    LightArray<VkSemaphore> waitSemaphores;

    LightArray<VkPipelineStageFlags> waitPipelineStageFlags;
};

}
}
