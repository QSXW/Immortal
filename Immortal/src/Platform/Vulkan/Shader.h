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
        VkDescriptorSetLayout   descriptorSetLayout;
        PipelineLayout          pipelineLayout;
        std::shared_ptr<Buffer> buffer;
    };

public:
    Shader(Device *device, const std::string &filename, Type type = Type::Graphics);

    ~Shader();

    VkShaderModule Load(const std::string &filename, Stage stage);

    auto &Stages()
    {
        return stages;
    }

    VkShaderStageFlags ConvertTo(Stage stage)
    {
        if (stage == Stage::Fragment)
        {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (stage == Stage::Compute)
        {
            return VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return VK_SHADER_STAGE_VERTEX_BIT;
    }

private:
    void Reflect(const std::string &source, std::vector<Resource> &resources, Stage stage);

    void INITUniform(const Resource &resource, Stage stage);

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    std::vector<Shader::Resource> resources;

    std::vector<Uniform> uniforms;
};

}
}
