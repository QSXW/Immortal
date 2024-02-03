#include "Shader.h"
#include "Framework/Utils.h"
#include "FileSystem/FileSystem.h"
#include "Graphics/DirectXShaderCompiler.h"
#include "Device.h"
#include <spirv_msl.hpp>

namespace Immortal
{
namespace Metal
{

Shader::Shader() :
    Handle<MTL::Library>{},
    function{},
    name{},
    stage{},
    threadGroupSize{}
{

}

Shader::Shader(Device *device, const std::string &_name, Stage stage, const std::string &source, const std::string &entryPoint) :
    Handle<MTL::Library>{},
    function{},
    name{},
    stage{ stage },
    threadGroupSize{}
{
    NS::StringEncoding encoding = NS::StringEncoding::ASCIIStringEncoding;
    std::string msl = GetMetalShaderSource(source, _name, stage, entryPoint);
    MTL::CompileOptions *compilerOptions = MTL::CompileOptions::alloc();
    compilerOptions->setLanguageVersion(MTL::LanguageVersion3_1);

    NS::String *str = NS::String::string(msl.c_str(), encoding);

    NS::Error *error = nullptr;
    handle = device->GetHandle()->newLibrary(str, compilerOptions, &error);
    CheckError(error);
    str->release();
    compilerOptions->release();

    NS::String *functionName = NS::String::string(name.c_str(), encoding);
    MTL::FunctionDescriptor *functionDescriptor = MTL::FunctionDescriptor::functionDescriptor();
    functionDescriptor->setName(functionName);
    functionName->release();

    function = handle->newFunction(functionDescriptor, &error);
    CheckError(error);
}

Shader::~Shader()
{
    if (function)
    {
        function->release();
        function = {};
    }
}

std::string Shader::GetMetalShaderSource(const std::string &source, const std::string &_name, ShaderStage stage, const std::string &entryPoint)
{
    DirectXShaderCompiler directXShaderCompiler;
    std::string error;
    std::vector<uint8_t> spirv;
    if (!(directXShaderCompiler.Compile(
            _name,
            ShaderSourceType::HLSL,
            ShaderBinaryType::SPIRV,
            stage,
            source.size(),
            source.c_str(),
            entryPoint,
            spirv,
            error))
        )
    {
        LOG::ERR("Shader compiling failing: \n\n{}", error);
        throw std::runtime_error("Compiler Error");
    }

    std::unique_ptr<spirv_cross::CompilerMSL> compilerMSL;
    compilerMSL.reset(new spirv_cross::CompilerMSL((uint32_t *)spirv.data(), spirv.size() >> 2));
    spirv_cross::CompilerMSL::Options options = compilerMSL->get_msl_options();
    options.make_msl_version(3, 1);
    compilerMSL->set_msl_options(options);

    auto msl = compilerMSL->compile();
    if (stage == ShaderStage::Compute)
    {
        const spirv_cross::SPIREntryPoint &_entryPoint = compilerMSL->get_entry_point(entryPoint, spv::ExecutionModelGLCompute);
        threadGroupSize.x = _entryPoint.workgroup_size.x;
        threadGroupSize.y = _entryPoint.workgroup_size.y;
        threadGroupSize.z = _entryPoint.workgroup_size.z;
        name = _entryPoint.name;
    }
    else
    {
        name = entryPoint;
    }

    return msl;
}

}
}
