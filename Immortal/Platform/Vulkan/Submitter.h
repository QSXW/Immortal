#pragma once

#include "Common.h"
#include "CommandBuffer.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace Vulkan
{

struct Timeline
{
    VkSemaphore semaphore;
    uint64_t value;
};

struct TimelineSubmitter
{
    TimelineSubmitter() :
        submitInfo{}
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    }

    void Wait(uint64_t value)
    {
#ifdef _DEBUG
        SLASSERT(submitted == false && "You've already submit the info. The change will not valid!");
#endif
        waitValues.emplace_back(value);
    }

    void Signal(uint64_t value)
    {
#ifdef _DEBUG
        SLASSERT(submitted == false && "You've already submit the info. The change will not valid!");
#endif
        signalValues.emplace_back(value);
    }

    operator const VkTimelineSemaphoreSubmitInfo&()
    {
#ifdef _DEBUG
        SLASSERT(submitted == false && "It's illegal to submit the same info again!");
        submitted = true;
#endif

        submitInfo.waitSemaphoreValueCount = U32(waitValues.size());
        submitInfo.pWaitSemaphoreValues    = waitValues.data();

        submitInfo.signalSemaphoreValueCount = U32(signalValues.size());
        submitInfo.pSignalSemaphoreValues    = signalValues.data();

        return submitInfo;
    }

private:
    VkTimelineSemaphoreSubmitInfo submitInfo;

    LightArray<uint64_t> waitValues;

    LightArray<uint64_t> signalValues;

#ifdef _DEBUG
    bool submitted = false;
#endif // _DEBUG
};

struct Submitter 
{
    Submitter() :
        submitInfo{}
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        ppNext = (void **)&submitInfo.pNext;
    }

    void WaitSemaphore(const VkSemaphore semaphore, VkPipelineStageFlags pipelineStageFlags)
    {
        waitSemaphores.emplace_back(semaphore);
        waitPipelineStages.emplace_back(pipelineStageFlags);
    }

    void SignalSemaphore(const VkSemaphore semaphore)
    {
        signalSemaphores.emplace_back(semaphore);
    }

    void Execute(const VkCommandBuffer commandbuffer)
    {
        commandBuffers.emplace_back(commandbuffer);
    }

    void Execute(const CommandBuffer *commandBuffer)
    {
        Execute(*commandBuffer);
    }

    void Clear()
    {
        waitSemaphores.clear();
        signalSemaphores.clear();
        commandBuffers.clear();
    }

    template <class T>
    void Trampoline(const T *pNext)
    {
        *ppNext = (void *)pNext;
         ppNext = (void**)&(pNext->pNext);
    }

    void Trampoline(TimelineSubmitter &timelineSubmitter)
    {
        const VkTimelineSemaphoreSubmitInfo &timelineSemaphoreSubmitInfo = timelineSubmitter;
        Trampoline(&timelineSemaphoreSubmitInfo);
    }

    operator const VkSubmitInfo& ()
    {
        submitInfo.waitSemaphoreCount = U32(waitSemaphores.size());
        submitInfo.pWaitSemaphores    = waitSemaphores.data();

        submitInfo.signalSemaphoreCount = U32(signalSemaphores.size());
        submitInfo.pSignalSemaphores    = signalSemaphores.data();

        submitInfo.commandBufferCount = U32(commandBuffers.size());
        submitInfo.pCommandBuffers    = commandBuffers.data();

        submitInfo.pWaitDstStageMask = waitPipelineStages.data();

        return submitInfo;
    }

private:
    VkSubmitInfo submitInfo;

    LightArray<VkSemaphore> waitSemaphores{};

    LightArray<VkSemaphore> signalSemaphores{};

    LightArray<VkCommandBuffer> commandBuffers{};

    LightArray<VkPipelineStageFlags> waitPipelineStages{};

    void **ppNext = nullptr;
};

class PresentSubmitter 
{
public:
    PresentSubmitter() :
        handle{}
    {
        handle.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    }

    void Wait(VkSemaphore semaphore)
    {
        waitSemaphores.emplace_back(semaphore);
    }

    void Present(VkSwapchainKHR swapchain, uint32_t imageIndex)
    {
        swapchains.emplace_back(swapchain);
        imageIndices[swapchains.size() - 1] = imageIndex;
    }

    operator const VkPresentInfoKHR&()
    {
        handle.waitSemaphoreCount = U32(waitSemaphores.size());
        handle.pWaitSemaphores    = waitSemaphores.data();

        handle.swapchainCount = U32(swapchains.size());
        handle.pSwapchains    = swapchains.data();
        handle.pImageIndices  = imageIndices.data();

        return handle;
    }

    void clear()
    {
        waitSemaphores.clear();

        swapchains.clear();
    }

private:
    VkPresentInfoKHR handle;

    LightArray<VkSemaphore, 3> waitSemaphores;

    LightArray<VkSwapchainKHR, 3> swapchains;
   
    std::array<uint32_t, 3> imageIndices;
};

}
}
