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

public:
    Queue(Device *device, uint32_t familyIndex, VkQueueFamilyProperties properties, VkBool32 canPresent, uint32_t index);

    Queue(const Queue &) = default;

    Queue(Queue &&other);

    virtual ~Queue();

    VkResult Submit(VkSubmitInfo &submitInfo, VkFence fence)
    {
        return vkQueueSubmit(handle, 1, &submitInfo, fence);
    }

    void Wait()
    {
        vkQueueWaitIdle(handle);
    }

    template <class T>
    T &Get()
    {
        if constexpr (IsPrimitiveOf<FamilyIndex, T>())
        {
            return familyIndex;
        }
    }

    VkQueue &Handle()
    {
        return handle;
    }

    operator VkQueue&()
    {
        return handle;
    }

    operator VkQueue() const
    {
        return handle;
    }

    VkQueueFamilyProperties &Properties()
    {
        return properties;
    }

    VkBool32 &IsPresentSupported()
    {
        return presented;
    }

    VkResult Present(VkPresentInfoKHR &presentInfo)
    {
        if (!presented)
        {
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
        }

        return vkQueuePresentKHR(handle, &presentInfo);
    }

private:
    Device *device;

    VkQueue handle{ VK_NULL_HANDLE };

    uint32_t familyIndex{ 0 };

    uint32_t index{ 0 };

    VkBool32 presented{ VK_FALSE };

    VkQueueFamilyProperties properties{};

private:
    //Queue &operator=(const Queue &) = delete;
    Queue &operator=(Queue &&) = delete;
};
}
}
