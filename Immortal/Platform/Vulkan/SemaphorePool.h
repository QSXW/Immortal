#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class SemaphorePool
{
public:
	SemaphorePool();

    SemaphorePool(Device *device);

    ~SemaphorePool();

    VkSemaphore Request();

    VkSemaphore Request(uint64_t initialValue);

    void Reset();

private:
    VkSemaphore InternalRequest(const VkSemaphoreCreateInfo &createInfo);

private:
    Device *device{ nullptr };

    std::vector<VkSemaphore> handles;

    uint32_t activeCount{ 0 };
};

}
}
