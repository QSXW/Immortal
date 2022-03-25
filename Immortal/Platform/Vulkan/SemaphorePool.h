#pragma once

#include "Common.h"
#include "Sync/Semaphore.h"
#include "Sync/SemaphorePool.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class SemaphorePool : public SuperSemaphorePool
{
public:
    SemaphorePool(Device *device);

    ~SemaphorePool();

    VkSemaphore Request();

    void Reset();

private:
    Device *device{ nullptr };

    std::vector<VkSemaphore> handles;

    uint32_t activeCount{ 0 };
};

}
}
