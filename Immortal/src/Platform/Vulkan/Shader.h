#pragma once

#include <array>

#include "Common.h"
#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"

namespace Immortal
{
namespace Vulkan
{

class Device;
class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(Device *device, const std::string &filename, Shader::Type type = Shader::Type::Graphics);

    ~Shader();

    VkShaderModule Load(const std::string &filename, Shader::Stage stage);

    auto &Stages()
    {
        return stages;
    }

private:
    Device *device{ nullptr };

    std::vector<VkShaderModule> modules;

    std::vector<VkPipelineShaderStageCreateInfo> stages;
};

}
}
