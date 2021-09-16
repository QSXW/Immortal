#pragma once

#include "Render/Shader.h"
#include "Render/GLSLCompiler.h"

#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(Device *device, const std::string &filename);

    VkShaderModule Load(const std::string &filename, Shader::Stage stage);

private:
    Device *device{ nullptr };
};

}
}
