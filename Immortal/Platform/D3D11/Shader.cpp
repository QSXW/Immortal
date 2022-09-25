#include "Shader.h"
#include "Framework/Utils.h"
#include "FileSystem/FileSystem.h"
#include "Device.h"

#include <d3d11shader.h>

namespace Immortal
{
namespace D3D11
{

Shader::Shader(Device *device, const std::string &filepath, Type type) :
    Super{ type },
    handles{}
{
	std::string path = filepath + ".hlsl";
	auto source = FileSystem::ReadString(path);
	LoadByteCodes(device, source, path, type);
}

Shader::Shader(Device *device, const std::string &name, const std::string &source, Type type) :
    Super{ type },
    handles{}
{
	LoadByteCodes(device, source, name);
}

Shader::~Shader()
{

}

void Shader::LoadByteCodes(Device *device, const std::string &source, const std::string &name, Type type)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> error;
	const char *target = "vs_5_0";
	const char *entryPoint = "VSMain";

	if (type == Type::Compute)
	{
		target = "cs_5_0";
		entryPoint = "main";
	}

	ShaderByteCodes shaderByteCodes;

	D3D_SHADER_MACRO defines[] = {
	    { .Name = "__D3D11__", .Definition = NULL  },
	    { .Name = NULL,        .Definition = NULL  },
	};

	__Check(
	    D3DCompile(
	        source.c_str(),
	        source.size(),
	        name.c_str(),
	        defines,
	        nullptr,
	        entryPoint,
	        target,
	        compileFlags,
	        0,
	        &shaderByteCodes.Graphics.Vertex,
	        &error),
	    &error,
	    &shaderByteCodes.Graphics.Vertex);

	if (type == Type::Graphics)
	{
		__Check(
		    D3DCompile(
		        source.c_str(),
		        source.size(),
		        name.c_str(),
		        defines,
		        nullptr,
		        "PSMain",
		        "ps_5_0",
		        compileFlags,
		        0,
		        &shaderByteCodes.Graphics.Pixel,
		        &error),
		    &error,
		    &shaderByteCodes.Graphics.Pixel);

		VSByteCodes = shaderByteCodes.Graphics.Vertex;
		__Create<ID3D11VertexShader, 0>(device, shaderByteCodes.Graphics.Vertex.Get());
		__Create<ID3D11PixelShader,  1>(device, shaderByteCodes.Graphics.Pixel.Get());
		
		__Reflect(shaderByteCodes.Graphics.Vertex.Get());
		__Reflect(shaderByteCodes.Graphics.Pixel.Get());
	}
	else
	{
		__Create<ID3D11ComputeShader, 0>(device, shaderByteCodes.Compute.Get());
		__Reflect(shaderByteCodes.Compute.Get());
	}
}

void Shader::__Check(HRESULT result, ID3DBlob **error, ID3DBlob **toBeReleased)
{
    if (FAILED(result) || !*toBeReleased)
    {
        if (*error)
        {
            LOG::ERR("D3D12 Shader Compiling failed with...\n{}", rcast<char *>((*error)->GetBufferPointer()));
        }

        if (*toBeReleased)
        {
            (*toBeReleased)->Release();
            *toBeReleased = nullptr;
        }

        SLASSERT(false && "Failed to compile shader source");
    }
}

void Shader::__Reflect(ID3DBlob *byteCodes)
{
	ComPtr<ID3D11ShaderReflection> reflection = NULL;
	Check(D3DReflect(byteCodes->GetBufferPointer(), byteCodes->GetBufferSize(), IID_PPV_ARGS(&reflection)));

	D3D11_SHADER_DESC desc;
	Check(reflection->GetDesc(&desc));
	for (size_t i = 0; i < desc.ConstantBuffers; i++)
	{
		auto buffer = reflection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		Check(buffer->GetDesc(&bufferDesc));

		if (!strcmp(bufferDesc.Name, "push_constant"))
		{
			pushConstants.Binding = i;
			pushConstants.Size = bufferDesc.Size;
		}
	}
}

}
}
