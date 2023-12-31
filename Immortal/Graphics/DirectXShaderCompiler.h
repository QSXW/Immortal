#pragma once

#include "Shader.h"
#include "Shared/DLLLoader.h"

class IDxcUtils;
class IDxcCompiler3;
class ID3D12ShaderReflection;

namespace Immortal
{

class DirectXShaderCompiler
{
public:
	DirectXShaderCompiler();

	~DirectXShaderCompiler();

	bool Compile(const std::string     &name, 
					    ShaderSourceType       sourceType,
	                    ShaderBinaryType       binaryType,
	                    ShaderStage            stage,
	                    uint32_t               size,
	                    const char            *data,
	                    const std::string     &entryPoint,
	                    std::vector<uint8_t>  &binary,
	                    std::string           &error);

	bool Reflect(ShaderBinaryType binaryType, const std::vector<uint8_t> &binary, REFIID iid, void **ppvReflection);

protected:
	DLLLoader dxc;

	IDxcUtils *utils;

	IDxcCompiler3 *compiler;
};

};
