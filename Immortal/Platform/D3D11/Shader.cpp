#include "Shader.h"
#include "Framework/Utils.h"
#include "FileSystem/FileSystem.h"
#include "Device.h"
#include <d3d11shader.h>

namespace Immortal
{
namespace D3D11
{

static const char *GetShaderTarget(ShaderStage stage)
{
	switch (stage)
	{
		case ShaderStage::Vertex:
			return "vs_5_0";
		case ShaderStage::Pixel:
			return "ps_5_0";
		case ShaderStage::Compute:
			return "cs_5_0";
		default:
			return nullptr;
	}
}

Shader::Shader() :
    NonDispatchableHandle{},
    handle{},
    stage{},
    pushConstant{}
{

}

Shader::Shader(Device *device, const std::string &name, ShaderSourceType sourceType, ShaderStage stage, const std::string &source, const std::string &entryPoint) :
    NonDispatchableHandle{ device },
    handle{},
    stage{ stage },
    pushConstant{}
{
	LoadByteCodesFromSource(name, stage, source, entryPoint);

	auto size       = byteCodes->GetBufferSize();
	auto pByteCodes = byteCodes->GetBufferPointer();

	ComPtr<ID3D11ShaderReflection> shaderReflection;
	Check(D3DReflect(pByteCodes, size, IID_PPV_ARGS(&shaderReflection)));
	Reflect(shaderReflection.Get());
	ConstructHandle(pByteCodes, size);
}

void Shader::LoadByteCodesFromSource(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> error;
	D3D_SHADER_MACRO defines[] = {
	    { .Name = "__D3D11__", .Definition = NULL  },
	    { .Name = NULL,        .Definition = NULL  },
	};

	if (FAILED(D3DCompile(
		source.c_str(),
		source.size(),
		name.c_str(),
		defines,
		nullptr,
		entryPoint.c_str(),
		GetShaderTarget(stage),
		compileFlags,
		0,
		&byteCodes,
		&error)
	))
	{
		LOG::ERR("Shader `{}` Compiling Error: \n\n{}", name, (const char *)error->GetBufferPointer());
		throw std::runtime_error("Failed to compiler shader!");
	}
}

Shader::~Shader()
{
	handle.Reset();
}

void Shader::Reflect(ID3D11ShaderReflection *shaderRelection)
{
	D3D11_SHADER_DESC desc;
	Check(shaderRelection->GetDesc(&desc));
	for (size_t i = 0; i < desc.ConstantBuffers; i++)
	{
		auto buffer = shaderRelection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		Check(buffer->GetDesc(&bufferDesc));

		if (!strcmp(bufferDesc.Name, "push_constant") ||
			!strcmp(bufferDesc.Name, "$Globals"))
		{
			pushConstant.Binding = i;
			pushConstant.Size = bufferDesc.Size;
		}
	}
}

void Shader::ConstructHandle(const void *bufferPointer, size_t bufferSize)
{
	switch (stage)
	{
		case ShaderStage::Vertex:
		{
			ComPtr<ID3D11VertexShader> pShader;
			Check(device->CreateVertexShader(bufferPointer, bufferSize, nullptr, &pShader));
			pShader.As<ID3D11DeviceChild>(&handle);
			break;
		}
		case ShaderStage::Pixel:
		{
			ComPtr<ID3D11PixelShader> pShader;
			Check(device->CreatePixelShader(bufferPointer, bufferSize, nullptr, &pShader));
			pShader.As<ID3D11DeviceChild>(&handle);
			break;
		}
		case ShaderStage::Compute:
		{
			ComPtr<ID3D11ComputeShader> pShader;
			Check(device->CreateComputeShader(bufferPointer, bufferSize, nullptr, &pShader));
			pShader.As<ID3D11DeviceChild>(&handle);
			break;
		}
		default:
			break;
	}
}

}
}
