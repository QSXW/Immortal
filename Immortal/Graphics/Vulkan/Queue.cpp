#include "Queue.h"
#include "Device.h"
#include "CommandBuffer.h"
#include "GPUEvent.h"

namespace Immortal
{
namespace Vulkan
{

DeviceQueue::DeviceQueue(Device *device) :
    device{ device },
    familyIndex{},
    index{},
    presented{ VK_TRUE },
    properties{}
{

}

DeviceQueue::DeviceQueue(Device *device, uint32_t familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, uint32_t index) :
    device{ device },
    familyIndex{ familyIndex },
    index{ index },
    presented{ VK_TRUE },
    properties{ properties }
{
    device->GetQueue(familyIndex, index, &handle);
}

DeviceQueue::~DeviceQueue()
{
	handle = nullptr;
	device = nullptr;
}

Queue::Queue(DeviceQueue *queue) :
    queue{ queue },
    executionCompleteSemaphores{}
{

}

Queue::~Queue()
{
    if (queue)
    {
        for (auto &semaphore : executionCompleteSemaphores)
        {
            if (semaphore)
            {
			    queue->GetDevice()->Release(std::move(semaphore));
            }
        }
		queue->DeOccupy();
    }
}

void Queue::WaitIdle(uint32_t timeout)
{
	(void)timeout;
	Check(queue->WaitIdle());
}

void Queue::Wait(SuperGPUEvent *_pEvent)
{
	GPUEvent *pEvent = InterpretAs<GPUEvent>(_pEvent);
    if (!pEvent->IsTimeline())
    {
		waitSemaphores.emplace_back(pEvent->GetSemaphore());
		waitPipelineStageFlags.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }
}

void Queue::Signal(SuperGPUEvent *pEvent)
{

}

void Queue::Submit(SuperCommandBuffer **_ppCommandBuffer, size_t count, SuperGPUEvent **_ppSignalEvents, uint32_t eventCount, SuperSwapchain *_swapchain)
{
	LightArray<VkCommandBuffer> commandBuffers;
	commandBuffers.resize(count);

    CommandBuffer **ppCommandBuffer = (CommandBuffer **)_ppCommandBuffer;
    for (size_t i = 0; i < count; i++)
    {
		commandBuffers[i] = *ppCommandBuffer[i];
    }

    LightArray<VkSemaphore> signalSemaphores;
	signalSemaphores.resize(eventCount);

    LightArray<uint64_t> signalValues;
	signalValues.resize(eventCount);
    GPUEvent **ppSignalEvents = (GPUEvent **)_ppSignalEvents;
    for (size_t i = 0; i < eventCount; i++)
    {
		signalSemaphores[i] = *ppSignalEvents[i];
		signalValues[i] = ppSignalEvents[i]->GetIncrement();
    }

    LightArray<uint64_t> waitSemaphoreValues{};
	VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo{
        .sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .pNext                     = nullptr,
        .waitSemaphoreValueCount   = uint32_t(waitSemaphores.size()),
	    .pWaitSemaphoreValues      = waitSemaphoreValues.data(),
        .signalSemaphoreValueCount = eventCount,
        .pSignalSemaphoreValues    = signalValues.data()
    };

    if (_swapchain)
    {
		Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
		waitSemaphores.emplace_back(swapchain->GetAcquiredImageReadySemaphore());
		waitPipelineStageFlags.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

	VkSubmitInfo submitInfo{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = &timelineSemaphoreSubmitInfo,
	    .waitSemaphoreCount   = uint32_t(waitSemaphores.size()),
        .pWaitSemaphores      = waitSemaphores.data(),
        .pWaitDstStageMask    = waitPipelineStageFlags.data(),
        .commandBufferCount   = uint32_t(count),
        .pCommandBuffers      = commandBuffers.data(),
        .signalSemaphoreCount = eventCount,
        .pSignalSemaphores    = signalSemaphores.data()
    };

    Check(queue->Submit(1, &submitInfo, VK_NULL_HANDLE));

    waitSemaphores.resize(0);
	waitPipelineStageFlags.resize(0);
}

void Queue::Present(SuperSwapchain *_swapchain, SuperGPUEvent **_ppSignalEvents, uint32_t eventCount)
{
	Swapchain *swapchain = InterpretAs<Swapchain>(_swapchain);
    
    uint32_t bufferIndex = swapchain->GetCurrentBufferIndex();
  //  if (executionCompleteSemaphores.size() <= bufferIndex)
  //  {
		//Device *device = queue->GetDevice();
		//executionCompleteSemaphores.resize(bufferIndex + 1);
		//Check(device->AllocateSemaphore(&executionCompleteSemaphores[bufferIndex]));
  //  }

    uint32_t bufferIndices[] = { bufferIndex };
    VkSwapchainKHR swapchains[] = { *swapchain };
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 0,// 1,
        .pWaitSemaphores    = nullptr,
	    .swapchainCount     = SL_ARRAY_LENGTH(swapchains),
        .pSwapchains        = swapchains,
        .pImageIndices      = bufferIndices,
        .pResults           = nullptr,
    };

    VkResult result = queue->Present(&presentInfo);
    if (result != VK_ERROR_OUT_OF_DATE_KHR)
    {
		Check(result);
    }

    swapchain->OnPresent();
}

}
}
