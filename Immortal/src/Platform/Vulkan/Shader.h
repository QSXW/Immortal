#pragma once

#include <array>

#include "Common.h"
#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"
#include "PipelineLayout.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Shader : public SuperShader
{
public:
    using Super = SuperShader;

    struct Uniform
    {
        VkDescriptorSet         descriptorSet;
        std::shared_ptr<Buffer> buffer;
    };

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
        if (IsPrimitiveOf<PipelineLayout, T>())
        {
            return pipelineLayout;
        }
    }

private:
    void Reflect(const std::string &source, std::vector<Resource> &resources, Stage stage);

    void BuildUniformBuffer(const Resource &resource, Stage stage);

    void INIT();

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    std::vector<Shader::Resource> resources;

    std::vector<Uniform> uniforms;

    PipelineLayout pipelineLayout;

    VkDescriptorSetLayout descriptorSetLayout;

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    std::vector<VkPushConstantRange> pushConstantRanges;
};

}
}
