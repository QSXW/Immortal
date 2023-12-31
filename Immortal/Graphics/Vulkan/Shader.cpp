#include "Shader.h"

#include <stack>

#include "FileSystem/FileSystem.h"
#include "Graphics/GLSLCompiler.h"
#include "Device.h"
#include "FileSystem/RF.h"
#include "Shared/DLLLoader.h"
#ifdef _WIN32
#include "Graphics/DirectXShaderCompiler.h"
#endif

namespace Immortal
{
namespace Vulkan
{

void CacheSpirv(const std::string &path, const std::string &shaderName, const std::string &src, const std::vector<uint8_t> &spirv)
{
    std::string filename = std::to_string(std::hash<std::string>{}(shaderName)) + std::string{ ".spirv" };

    if (!FileSystem::Exists(path))
    {
        FileSystem::CreateDirectory(path);
    }

    RF rf{ FileSystem::Join(path, filename), Stream::Mode::Write };

    if (!rf.Writable())
    {
        return;
    }

    size_t srcHash = std::hash<std::string>{}(src);
    rf.Append(&srcHash);
    rf.Append(spirv);
    rf.Write();
}

bool ReadSpirv(const std::string &path, const std::string &shaderName, const std::string &src, std::vector<uint8_t> &spirv)
{
    std::string filename = std::to_string(std::hash<std::string>{}(shaderName)) + std::string{ ".spirv" };

    if (!FileSystem::Exists(path))
    {
        FileSystem::CreateDirectory(path);
    }

    RF rf{ FileSystem::Join(path, filename), Stream::Mode::Read };
    if (!rf.Readable())
    {
        return false;
    }

    auto &chunks = rf.Read();
    if (chunks.size() != 2)
    {
        LOG::WARN("Cached spirv file corrupted!");
        return false;
    }
    auto hash = *(const size_t *)(chunks[0].ptr);
    if (hash != std::hash<std::string>{}(src))
    {
        return false;
    }

    spirv.resize(chunks[1].size);
    memcpy(spirv.data(), chunks[1].ptr, chunks[1].size);

    return true;
}

static inline VkShaderStageFlagBits CAST(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Mesh:
            return VK_SHADER_STAGE_MESH_BIT_EXT;
        case ShaderStage::Pixel:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TesselationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TesselationControl:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_ALL;
    }
}

Shader::Shader(Device *device, const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) :
    Super{},
    device{ device },
    pushConstantRanges{},
    descriptorSetLayoutBindings{},
    entryPoint{ entryPoint },
    stage{ CAST(stage) }
{
    handle = Load(name, stage, source, entryPoint);
}

Shader::~Shader()
{
    if (device)
    {
        device->DestroyAsync(handle);
        handle = VK_NULL_HANDLE;
        device = nullptr;
    }
}

VkPipelineShaderStageCreateInfo Shader::CreateStage(VkShaderModule module, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo createInfo{};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = module;
    createInfo.pName  = entryPoint.c_str();
    return createInfo;
}

VkShaderModule Shader::Load(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint)
{
    constexpr const char *tmpPath = "SpirvCache/";

    std::vector<uint8_t> spirv;
    std::string error;

    if (!ReadSpirv(tmpPath, name, source, spirv))
    {
		DirectXShaderCompiler compiler{};
		if (!compiler.Compile(name, ShaderSourceType::HLSL, ShaderBinaryType::SPIRV, stage, source.size(), source.data(), entryPoint.c_str(), spirv, error))
        {
            LOG::FATAL("Failed to compiler Shader => {0}\n{1}", name, error);

            throw RuntimeException(error.c_str());
            return VK_NULL_HANDLE;
        }
        CacheSpirv(tmpPath, name, source, spirv);
    }

    return CreateModuleBySpriv(spirv, CAST(stage));
}

VkShaderModule Shader::Load(const std::string &filename, Shader::Stage stage)
{
    auto source = FileSystem::ReadString(filename);
    return Load(filename, stage, source, "main");
}

VkShaderModule Shader::CreateModuleBySpriv(const std::vector<uint8_t> &spirv, VkShaderStageFlagBits stage)
{
    SPRIVReflector::Reflect(spirv, descriptorSetLayoutBindings, pushConstantRanges);

    VkShaderModule shaderModule{ VK_NULL_HANDLE };
    VkShaderModuleCreateInfo createInfo = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = {},
        .codeSize = spirv.size(),
        .pCode    = (const uint32_t *)spirv.data(),
    };

    Check(device->Create(&createInfo, &shaderModule));

    for (auto &descriptorSetLayoutBinding : descriptorSetLayoutBindings)
    {
		descriptorSetLayoutBinding.stageFlags = stage;
    }
    for (auto &pushConstantRanges : pushConstantRanges)
    {
        pushConstantRanges.stageFlags = stage;
    }

    return shaderModule;
}

}
}
