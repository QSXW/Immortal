#pragma once

#include <array>

#include "Common.h"
#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"
#include "PipelineLayout.h"
#include "Buffer.h"
#include "Descriptor.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Shader : public SuperShader
{
public:
    using Super = SuperShader;

    static VkShaderStageFlags ConvertTo(Stage stage)
    {
        static VkShaderStageFlags map[] = {
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_COMPUTE_BIT,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
            VK_SHADER_STAGE_MISS_BIT_KHR,
            VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
            VK_SHADER_STAGE_CALLABLE_BIT_KHR
        };
        return map[ncast<size_t>(stage)];
    }

public:
    Shader(Device *device, const std::string &filename, Type type = Type::Graphics);

    ~Shader();

    VkShaderModule Load(const std::string &filename, Stage stage);

    auto &Stages()
    {
        return stages;
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<PipelineLayout, T>())
        {
            return pipelineLayout;
        }
        if constexpr (IsPrimitiveOf<VkDescriptorSet, T>())
        {
            return descriptorSet;
        }
    }

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<DescriptorSetUpdater, T>())
        {
            return &descriptorSetUpdater;
        }
    }

private:
    void Reflect(const std::string &source, std::vector<Resource> &resources, Stage stage);

    void Setup();

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    std::vector<Shader::Resource> resources;

    PipelineLayout pipelineLayout;

    VkDescriptorSetLayout descriptorSetLayout;

    DescriptorSetUpdater descriptorSetUpdater;

    VkDescriptorSet descriptorSet;

    std::vector<VkPushConstantRange> pushConstantRanges;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
};

}
}
