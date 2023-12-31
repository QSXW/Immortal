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

void CacheSpirv(const std::string &path, const std::string &shaderName, const std::string &src, const std::vector<uint8_t> &spirv, uint32_t identifierSize, const uint8_t *identifier)
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
	rf.Append(&identifierSize);
	rf.Append(identifier, identifierSize);
    rf.Write();
}

bool ReadSpirv(const std::string &path, const std::string &shaderName, const std::string &src, std::vector<uint8_t> &spirv, uint32_t *pIdentifierSize, uint8_t *identifier)
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
    if (chunks.size() < 2 || chunks.size() > 4)
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

    if (chunks.size() == 4)
    {
		*pIdentifierSize = *(const uint32_t *)(chunks[2].ptr);
		memcpy(identifier, chunks[3].ptr, chunks[3].size);
    }

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
    stage{ CAST(stage) },
    identifierSize{},
    identifier{}
{
    Load(name, stage, source, entryPoint);
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

void Shader::GetPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo *pPipelineShaderStageCreateInfo, VkPipelineShaderStageModuleIdentifierCreateInfoEXT *pIdentifierCreateInfo)
{
    VkPipelineShaderStageCreateInfo createInfo{
	    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .pNext               = nullptr,
	    .flags               = 0,
	    .stage               = GetStage(),
	    .module              = handle,
	    .pName               = GetEntryPoint(),
	    .pSpecializationInfo = nullptr,
    };

    if (identifierSize)
    {
		createInfo.module = VK_NULL_HANDLE;
		VkPipelineShaderStageModuleIdentifierCreateInfoEXT pipelineShaderStageModuleIdentifierCreateInfo = {
            .sType          = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT,
            .pNext          = nullptr,
            .identifierSize = identifierSize,
            .pIdentifier    = identifier
		};
 
		*pIdentifierCreateInfo = pipelineShaderStageModuleIdentifierCreateInfo;
		createInfo.pNext = (void *)pIdentifierCreateInfo;
    }
    
    *pPipelineShaderStageCreateInfo = createInfo;
}

void Shader::Load(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint)
{
    constexpr const char *tmpPath = "SpirvCache/";

    std::vector<uint8_t> spirv;
    std::string error;

	VkShaderModuleCreateInfo createInfo = {
	    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	    .pNext    = nullptr,
	    .flags    = {},
	    .codeSize = 0,
	    .pCode    = nullptr,
	};

    if (!ReadSpirv(tmpPath, name, source, spirv, &identifierSize, identifier))
    {
		DirectXShaderCompiler compiler{};
		if (!compiler.Compile(name, ShaderSourceType::HLSL, ShaderBinaryType::SPIRV, stage, source.size(), source.data(), entryPoint.c_str(), spirv, error))
        {
            LOG::FATAL("Failed to compiler Shader => {0}\n{1}", name, error);

            throw RuntimeException(error.c_str());
            return;
        }
        
        createInfo.codeSize = spirv.size();
		createInfo.pCode = (const uint32_t *) spirv.data();
        VkShaderModuleIdentifierEXT shaderModuleIdentifier{VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT};
        if (device->IsEnabled(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME))
        {
		    device->GetShaderModuleCreateInfoIdentifierEXT(&createInfo, &shaderModuleIdentifier);
        }

        CacheSpirv(tmpPath, name, source, spirv, shaderModuleIdentifier.identifierSize, shaderModuleIdentifier.identifier);
    }
	createInfo.codeSize = spirv.size();
	createInfo.pCode    = (const uint32_t *)spirv.data();

    return Construct(createInfo);
}

void Shader::Construct(const VkShaderModuleCreateInfo &createInfo)
{
	SPRIVReflector::Reflect(createInfo.pCode, createInfo.codeSize, descriptorSetLayoutBindings, pushConstantRanges);

    if (!identifierSize)
    {
		Check(device->Create(&createInfo, &handle));
    }

    for (auto &descriptorSetLayoutBinding : descriptorSetLayoutBindings)
    {
		descriptorSetLayoutBinding.stageFlags = stage;
    }
    for (auto &pushConstantRanges : pushConstantRanges)
    {
        pushConstantRanges.stageFlags = stage;
    }
}

}
}
