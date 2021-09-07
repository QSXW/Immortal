#pragma once

#include "vkcommon.h"
#include "Sync/Semaphore.h"
#include "Sync/SemaphorePool.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class SemaphorePool : public Immortal::SemaphorePool
{
public:
    SemaphorePool(Device *device);
        
    ~SemaphorePool();

    Semaphore Request() override;

    void Reset() override;

private:
    Device *device{ nullptr };
        
    std::vector<VkSemaphore> handles;

    UINT32 activeCount{ 0 };
};

}
}
