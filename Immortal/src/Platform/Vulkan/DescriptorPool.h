#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{
class Device;
class DescriptorSetLayout;

class DescriptorPool
{
public:
    static constexpr UINT32 MaxSetsPerPool = 0x10;

public:
    DescriptorPool(Device &device, const DescriptorSetLayout &layout, UINT32 poolSize = MaxSetsPerPool);

    ~DescriptorPool();

private:
    Device &device;
        
    std::vector<VkDescriptorPoolSize> poolSize;

    std::vector<VkDescriptorPool> handles;

    std::vector<UINT32> setsCount;

    UINT32 currentIndex{ 0 };
};
}
}
