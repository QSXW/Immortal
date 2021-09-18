#include "Shader.h"

#include "io/FileSystem.h"
#include "Render/GLSLCompiler.h"

namespace Immortal
{
namespace Vulkan
{

static inline VkPipelineShaderStageCreateInfo CreateStage(VkShaderModule module, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo createInfo{};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = module;
    createInfo.pName  = "main";
    return createInfo;
}

Shader::Shader(Device *d, const std::string &filename, Shader::Type type)
    : device{ d }
{   
    if (type == Shader::Type::Graphics)
    {
        VkShaderModule vert = Load(filename + ".vert", Shader::Stage::Vertex);
        VkShaderModule frag = Load(filename + ".frag", Shader::Stage::Fragment);

        modules.push_back(vert);
        modules.push_back(frag);
        auto vertShaderStage = CreateStage(vert, VK_SHADER_STAGE_VERTEX_BIT);
        auto fragShaderStage = CreateStage(frag, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    else
    {
        VkShaderModule comp = Load(filename + ".comp", Shader::Stage::Compute);
        modules.push_back(comp);
    }
    
}

VkShaderModule Shader::Load(const std::string &filename, Shader::Stage stage)
{
    GLSLCompiler compiler{};

    auto src = FileSystem::ReadBinary(filename);

    std::vector<uint32_t> spirv;
    std::string error;

    if (!GLSLCompiler::Src2Spirv(Shader::API::Vulkan, stage, src, "main", spirv, error))
    {
        LOG::FATAL("Failed to compiler Shader => {0}\n", error.c_str());
        return VK_NULL_HANDLE;
    }

    VkShaderModule shaderModule{ VK_NULL_HANDLE };
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode    = spirv.data();

    Check(vkCreateShaderModule(device->Handle(), &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

}
}
