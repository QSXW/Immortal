#include "GLSLCompiler.h"

#include "ImmortalCore.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>

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

};
