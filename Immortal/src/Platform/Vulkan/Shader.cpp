#include "Shader.h"

#include "io/FileSystem.h"
#include "Render/GLSLCompiler.h"

namespace Immortal
{
namespace Vulkan
{

Shader::Shader(Device *d, const std::string &filename)
    : device{ d }
{
    VkShaderModule vert = Load(filename + ".vert", Shader::Stage::Vertex);
    VkShaderModule frag = Load(filename + ".frag", Shader::Stage::Fragment);
}

VkShaderModule Shader::Load(const std::string &filename, Shader::Stage stage)
{
    GLSLCompiler compiler{};

    auto src = FileSystem::ReadBinary(filename);

    Buffer spirv;
    std::string error;

    if (!GLSLCompiler::Src2Spirv(Shader::API::Vulkan, stage, src, "main", spirv, error))
    {
        LOG::ERR("Failed to compiler Shader => {0}", error.c_str());
        return VK_NULL_HANDLE;
    }

    return VK_NULL_HANDLE;
}

}
}
