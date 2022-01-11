#include "GLSLCompiler.h"

#include "Core.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
#include <spirv_glsl.hpp>

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

bool GLSLCompiler::Src2Spirv(Shader::API api, Shader::Stage stage, uint32_t size, const char *data, const char *entryPoint, std::vector<uint32_t> &spriv, std::string &error)
{
    auto version = ncast<glslang::EShTargetLanguageVersion>(0);
    glslang::InitializeProcess();

    EShMessages messages = ncast<EShMessages>(EShMsgDefault | EShMsgSpvRules);
    
    if (api == Shader::API::Vulkan)
    {
        messages = ncast<EShMessages>(messages | EShMsgVulkanRules);
    }
    if (api == Shader::API::D3D12)
    {
        messages = ncast<EShMessages>(messages | EShMsgReadHlsl);
    }

    EShLanguage language = SelectLanguage(stage);

    const char *fileNameList[] = { "" };
    const char *source = data;
    const int length   = (int)size;
    glslang::TShader shader{ language };
    shader.setStringsWithLengthsAndNames(&source, &length, fileNameList, 1);
    shader.setEntryPoint(entryPoint);
    shader.setSourceEntryPoint(entryPoint);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetNone, version);

    if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
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

inline Shader::Resource GetDecoration(spirv_cross::CompilerGLSL &glsl, spirv_cross::Resource &resource)
{
    Shader::Resource ret;
    CleanUpObject(&ret);
    ret.set      = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
    ret.binding  = glsl.get_decoration(resource.id, spv::DecorationBinding);
    ret.location = glsl.get_decoration(resource.id, spv::DecorationLocation);
    ret.name     = std::string{ resource.name };

    return ret;
}

bool GLSLCompiler::Reflect(const std::vector<uint32_t> &spirv, std::vector<Shader::Resource> &resouces)
{
    auto glsl = new spirv_cross::CompilerGLSL{ spirv };
    if (!glsl)
    {
        throw RuntimeException(SError::OutOfMemory);
    }

    auto spirvResources = new spirv_cross::ShaderResources{ glsl->get_shader_resources() };
    if (!spirvResources)
    {
        throw RuntimeException(SError::OutOfMemory);
    }

    for (auto &spirvResource : spirvResources->sampled_images)
    {
        Shader::Resource resource = GetDecoration(*glsl, spirvResource);
        resource.type = Shader::Resource::Type::ImageSampler | Shader::Resource::Type::Uniform;

        spirv_cross::SPIRType type = glsl->get_type(spirvResource.type_id);
        resource.count = type.array.empty() ? 1 : type.array[0];

        resouces.emplace_back(std::move(resource));
    }
    for (auto &spirvResource : spirvResources->uniform_buffers)
    {
        Shader::Resource resource = GetDecoration(*glsl, spirvResource);
        resource.type = Shader::Resource::Type::Uniform;

        spirv_cross::SPIRType type = glsl->get_type(spirvResource.base_type_id);
        resource.size  = glsl->get_declared_struct_size(type);
        resource.count = 1; /* resource.member_count = type.member_types.size(); */

        resouces.emplace_back(std::move(resource));
    }
    for (auto &spirvResource : spirvResources->push_constant_buffers)
    {
        Shader::Resource resource = GetDecoration(*glsl, spirvResource);
        resource.type = Shader::Resource::Type::PushConstant;

        spirv_cross::SPIRType type = glsl->get_type(spirvResource.base_type_id);
        resource.size = glsl->get_declared_struct_size(type);

        resouces.emplace_back(std::move(resource));
    }

    delete spirvResources;
    delete glsl;

    return true;
}

};
