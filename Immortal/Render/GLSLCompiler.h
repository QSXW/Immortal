#pragma once

#include "Shader.h"
#include "vulkan/vulkan.h"

namespace Immortal
{

class GLSLCompiler
{
public:
	static bool Compile(const std::string &name,
	                    ShaderSourceType sourceType,
	                    ShaderBinaryType binaryType,
	                    ShaderStage stage,
	                    uint32_t size,
	                    const char *data,
	                    const std::string &entryPoint,
	                    std::vector<uint32_t> &spriv,
	                    std::string &error);
};

class SPRIVReflector
{
public:
	static bool Reflect(const std::vector<uint8_t> &spirv, std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, std::vector<VkPushConstantRange> &pushConstantRanges);
};

};
