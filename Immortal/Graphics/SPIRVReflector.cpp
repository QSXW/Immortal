#include "SPIRVReflector.h"

#include "Core.h"
#include <spirv_glsl.hpp>

namespace Immortal
{

static inline VkDescriptorSetLayoutBinding GetDescriptorSetLayout(spirv_cross::CompilerGLSL *glsl, spirv_cross::Resource &resource, VkDescriptorType descriptorType)
{
	spirv_cross::SPIRType type = glsl->get_type(resource.type_id);
	VkDescriptorSetLayoutBinding binding = {
		.binding            = glsl->get_decoration(resource.id, spv::DecorationBinding),
		.descriptorType     = descriptorType,
		.descriptorCount    = type.array.empty() ? 1 : type.array[0],
		.stageFlags         = {},
		.pImmutableSamplers = nullptr,
	};

	return binding;
}

bool SPRIVReflector::Reflect(const void *spriv, size_t size, std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, std::vector<VkPushConstantRange> &pushConstantRanges)
{
	URef<spirv_cross::CompilerGLSL> glsl{ new spirv_cross::CompilerGLSL{ (const uint32_t *) spriv, size / sizeof(uint32_t) } };
	if (!glsl)
	{
		return false;
	}

	URef<spirv_cross::ShaderResources> spirvResources{ new spirv_cross::ShaderResources{ glsl->get_shader_resources() } };
	if (!spirvResources)
	{
		return false;
	}

	for (auto &resource : spirvResources->uniform_buffers)
	{
		descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
	}
	for (auto &resource : spirvResources->separate_images)
	{
		descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE));
	}
	for (auto &resource : spirvResources->separate_samplers)
	{
		descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_SAMPLER));
	}
	for (auto &resource : spirvResources->storage_images)
	{
		descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE));
	}
	for (auto &resource : spirvResources->storage_buffers)
	{
		descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
	}
	for (auto &resource : spirvResources->push_constant_buffers)
	{
		spirv_cross::SPIRType type = glsl->get_type(resource.base_type_id);
		pushConstantRanges.emplace_back(VkPushConstantRange{
			.stageFlags = {},
			.offset     = 0,
			.size       = uint32_t(glsl->get_declared_struct_size(type)),
		});
	}

	return true;
}

};
