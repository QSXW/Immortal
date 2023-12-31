#include "GLSLCompiler.h"

#include "Core.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/OSDependent/osinclude.h>
#include <spirv_glsl.hpp>
#include "Shared/DLLLoader.h"

namespace Immortal
{

static inline EShLanguage SelectLanguage(Shader::Stage stage)
{
    if (stage == Shader::Stage::Fragment)
    {
        return EShLangFragment;
    }
    if (stage == Shader::Stage::Compute)
    {
        return EShLangCompute;
    }
    return EShLangVertex;
}

using PFN_DxcCreateInstance = HRESULT(*)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);

bool GLSLCompiler::Compile(const std::string &name, ShaderSourceType sourceType, ShaderBinaryType binaryType, ShaderStage stage, uint32_t size, const char *data, const std::string &entryPoint, std::vector<uint32_t> &spriv, std::string &error)
{
    using namespace glslang;
    EShTargetLanguageVersion version = {};
    glslang::InitializeProcess();

    EShMessages messages = EShMessages(EShMsgDefault | EShMsgSpvRules);
    EShSource glslangSourceType = EShSourceGlsl;

    if (sourceType == ShaderSourceType::GLSL)
    {
        messages = EShMessages(messages | EShMsgVulkanRules);
    }
    if (sourceType == ShaderSourceType::HLSL)
    {
        glslangSourceType = EShSourceHlsl;
        messages = EShMessages(messages | EShMsgReadHlsl);
    }

    EShLanguage language = SelectLanguage(stage);

    const char *fileNameList[] = { "" };
    const char *source = data;
    const int length   = (int)size;
    glslang::TShader shader{ language };
    shader.setStringsWithLengthsAndNames(&source, &length, fileNameList, 1);
    shader.setEntryPoint(entryPoint.c_str());
    shader.setSourceEntryPoint("main");
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, version);
    shader.setEnvInput(glslangSourceType, language, EShClientVulkan, version);
    shader.setPreamble("#define " PLATFORM_STRING "\n");

    auto defaultTBuiltInResource = GetDefaultResources();
    if (!shader.parse(defaultTBuiltInResource, 100, false, messages))
    {
        error = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        error = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    if (shader.getInfoLog())
    {
        error += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
    }
    if (program.getInfoLog())
    {
        error += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
    }

    glslang::TIntermediate *intermediate = program.getIntermediate(language);
    if (!intermediate)
    {
        error += "Failed to get shared intermediate code.\n";
        return false;
    }

    spv::SpvBuildLogger logger{};
    glslang::GlslangToSpv(*intermediate, spriv, &logger);
    error += logger.getAllMessages() + "\n";

    glslang::FinalizeProcess();

    return true;
}

inline VkDescriptorSetLayoutBinding GetDescriptorSetLayout(spirv_cross::CompilerGLSL *glsl, spirv_cross::Resource &resource, VkDescriptorType descriptorType)
{
    spirv_cross::SPIRType type = glsl->get_type(resource.type_id);
    VkDescriptorSetLayoutBinding binding = {
        .binding            = glsl->get_decoration(resource.id, spv::DecorationBinding),
        .descriptorType     = descriptorType,
        .descriptorCount    = type.array.empty() ? 1 : type.array[0],
        .stageFlags         = {},
        .pImmutableSamplers = nullptr,
    };

    return binding;
}

bool SPRIVReflector::Reflect(const void *spriv, size_t size, std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, std::vector<VkPushConstantRange> &pushConstantRanges)
{
	URef<spirv_cross::CompilerGLSL> glsl{new spirv_cross::CompilerGLSL{ (const uint32_t *) spriv, size / sizeof(uint32_t) }};
    if (!glsl)
    {
        return false;
    }

    URef<spirv_cross::ShaderResources> spirvResources{new spirv_cross::ShaderResources{glsl->get_shader_resources()}};
    if (!glsl)
    {
        return false;
    }

    for (auto &resource : spirvResources->uniform_buffers)
    {
        descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    }
    for (auto &resource : spirvResources->separate_images)
    {
        descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE));
    }
    for (auto &resource : spirvResources->separate_samplers)
    {
        descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_SAMPLER));
    }
    for (auto &resource : spirvResources->storage_images)
    {
        descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE));
    }
    for (auto &resource : spirvResources->storage_buffers)
    {
        descriptorSetLayoutBindings.emplace_back(GetDescriptorSetLayout(glsl, resource, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
    }
    for (auto &resource : spirvResources->push_constant_buffers)
    {
        spirv_cross::SPIRType type = glsl->get_type(resource.base_type_id);
        pushConstantRanges.emplace_back(VkPushConstantRange{
            .stageFlags = {},
            .offset     = 0,
            .size       = uint32_t(glsl->get_declared_struct_size(type)),
        });
    }

    return true;
}

};
