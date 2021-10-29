#pragma once

#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"

#include "Device.h"

#include <array>

namespace Immortal
{
namespace Vulkan
{

class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(Device *device, const std::string &filename, Shader::Type type = Shader::Type::Graphics);

    ~Shader();

    VkShaderModule Load(const std::string &filename, Shader::Stage stage);

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::array<VkPipelineShaderStageCreateInfo, 2> stages;
};

}
}
