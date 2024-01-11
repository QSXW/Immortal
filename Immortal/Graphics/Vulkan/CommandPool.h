#pragma once

#include "Common.h"
#include "Handle.h"
#include "Algorithm/LightArray.h"
#include "Shared/IObject.h"

#include <map>
#include <queue>

namespace Immortal
{
namespace Vulkan
{

class Device;
class CommandPool : public IObject, public Handle<VkCommandPool>
{
public:
    VKCPP_SWAPPABLE(CommandPool)

public:
    CommandPool(Device *device = nullptr, uint32_t threadId = 0, uint32_t queueFamilyIndex = 0, VkCommandPoolResetFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    ~CommandPool();

    VkResult Allocate(VkCommandBuffer *pCommandBuffer, uint32_t size, VkCommandBufferLevel level);

    void Release(VkCommandBuffer* pCommandBuffer, uint32_t size);

    void Destory();

public:
    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
    }

    void Swap(CommandPool &other)
    {
        Handle::Swap((Handle &) other);
        std::swap(device, other.device);
        std::swap(flags,  other.flags);
        std::swap(queueFamilyIndex, other.queueFamilyIndex);
    }

private:
    Device *device;

    VkCommandPoolResetFlags flags;

    uint32_t threadId;

    uint32_t queueFamilyIndex;
};

}
}
