#include "impch.h"

#include "DescriptorPool.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{
    DescriptorPool::DescriptorPool(Device &device, const DescriptorSetLayout &layout, UINT32 poolSize) :
        device{ device }
    {

    }

    DescriptorPool::~DescriptorPool()
    {
        for (auto &h : handles)
        {
            IfNotNullThen(vkDestroyDescriptorPool, device.Handle(), h, nullptr);
        }
    }
}
}
