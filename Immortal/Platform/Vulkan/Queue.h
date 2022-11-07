#pragma once

#include "Common.h"
#include "Render/Queue.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Queue : public SuperQueue
{
public:
    using FamilyIndex = uint32_t;
    using Super = SuperQueue;

    using Primitive = VkQueue;
    VKCPP_OPERATOR_HANDLE()

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
    Queue(Device *device, uint32_t familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, uint32_t index);

    Queue(const Queue &) = default;

    Queue(Queue &&other);

    virtual ~Queue();

    VkResult Submit(const VkSubmitInfo &submitInfo, VkFence fence)
    {
        return Submit(1, &submitInfo, fence);
    }

    template <class T>
    T &Get()
    {
        if constexpr (IsPrimitiveOf<FamilyIndex, T>())
        {
            return familyIndex;
        }
    }

    VkQueueFamilyProperties &Properties()
    {
        return properties;
    }

    VkBool32 &IsPresentSupported()
    {
        return presented;
    }

    VkResult Present(const VkPresentInfoKHR &presentInfo)
    {
        if (!presented)
        {
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
        }

        return PresentKHR(&presentInfo);
    }

private:
    Device *device;

    uint32_t familyIndex{ 0 };

    uint32_t index{ 0 };

    VkBool32 presented{ VK_FALSE };

    VkQueueFamilyProperties properties{};

private:
    Queue &operator=(const Queue &) = delete;
    Queue &operator=(Queue &&) = delete;
};

}
}
