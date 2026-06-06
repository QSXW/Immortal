#pragma once

#include <cstddef>
#include <vector>

#include "vulkan/vulkan.h"

namespace Immortal
{

class SPRIVReflector
{
public:
	static bool Reflect(const void *spriv, size_t size, std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, std::vector<VkPushConstantRange> &pushConstantRanges);
};

};
