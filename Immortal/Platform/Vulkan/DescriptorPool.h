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
    DescriptorPool(Device *device, const DescriptorSetLayout &layout, UINT32 poolSize = MaxSetsPerPool);

    DescriptorPool(Device *device, const std::vector<VkDescriptorPoolSize> &poolSize);

    ~DescriptorPool();

    VkResult Allocate(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSet, uint32_t count = 1);

    void Free(VkDescriptorSet *pDescriptorSet, uint32_t size = 1);

    void Reset();

private:
    Device *device{ nullptr };
        
    std::vector<VkDescriptorPoolSize> poolSize;

    VkDescriptorPool handle{ VK_NULL_HANDLE };

    std::vector<UINT32> setsCount;

    UINT32 currentIndex{ 0 };
};
}
}
