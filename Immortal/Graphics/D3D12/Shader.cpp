#include "Shader.h"
#include "Framework/Utils.h"
#include "FileSystem/FileSystem.h"
#include "Graphics/DirectXShaderCompiler.h"

namespace Immortal
{
namespace D3D12
{

static const char *GetShaderTarget(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return "vs_5_1";
        case ShaderStage::Pixel:
            return "ps_5_1";
        case ShaderStage::Compute:
            return "cs_5_1";
        default:
            return nullptr;
    }
}

static D3D12_SHADER_VISIBILITY CAST(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return D3D12_SHADER_VISIBILITY_VERTEX;
        case ShaderStage::Mesh:
            return D3D12_SHADER_VISIBILITY_MESH;
        case ShaderStage::Pixel:
            return D3D12_SHADER_VISIBILITY_PIXEL;
        case ShaderStage::Geometry:
            return D3D12_SHADER_VISIBILITY_GEOMETRY;
        case ShaderStage::TesselationEvaluation:
            return D3D12_SHADER_VISIBILITY_DOMAIN;
        case ShaderStage::TesselationControl:
            return D3D12_SHADER_VISIBILITY_HULL;
        default:
            return D3D12_SHADER_VISIBILITY_ALL;
    }
}

Shader::Shader(const std::string &name, Stage stage, const std::string &source, const std::string &entryPoint) :
    Super{},
    visibility{ CAST(stage) },
    pushConstants{},
    pushConstantIndex{}
{
    LoadByteCodes(source, name, stage, entryPoint);
}

Shader::Shader(Stage stage, ShaderBinaryType type, const void *binary, uint32_t size) :
    Super{},
    visibility{ CAST(stage) },
    pushConstants{},
    pushConstantIndex{}
{
    if (type != ShaderBinaryType::DXIL)
    {
		LOG::ERR("Unknown shader binary type. Only DXIL is supported!");
		return;
    }

    dxil.resize(size);
	memcpy(dxil.data(), binary, size);

	DirectXShaderCompiler directXShaderCompiler;
	ComPtr<ID3D12ShaderReflection> shaderReflection;
	if (directXShaderCompiler.Reflect(ShaderBinaryType::DXIL, dxil, IID_PPV_ARGS(&shaderReflection)))
	{
		SetupDescriptorRanges(shaderReflection);
		return;
	}
    else
    {
		Reflect();
    }
}

Shader::~Shader()
{

}

void Shader::LoadByteCodes(const std::string &source, const std::string &name, ShaderStage stage, const std::string &entryPoint)
{
    {
		DirectXShaderCompiler directXShaderCompiler;
        std::string error;
		if (!(directXShaderCompiler.Compile(
                name,
                ShaderSourceType::HLSL,
                ShaderBinaryType::DXIL,
                stage,
                source.size(),
                source.c_str(),
                entryPoint,
                dxil,
                error))
            )
        {
		    LOG::ERR("Shader compiling failing: \n\n{}", error);
		    throw std::runtime_error("Compiler Error");
        }

        ComPtr<ID3D12ShaderReflection> shaderReflection;
		if (directXShaderCompiler.Reflect(ShaderBinaryType::DXIL, dxil, IID_PPV_ARGS(&shaderReflection)))
        {
			SetupDescriptorRanges(shaderReflection);
			return;
        }
    }

    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    D3D_SHADER_MACRO defines[] = {
        {.Name = "__D3D12__",
         .Definition = NULL},
        {.Name = NULL,
         .Definition = NULL},
    };

    ComPtr<ID3DBlob> error;
	ComPtr<ID3DBlob> byteCodesBlob;
    if(FAILED(D3DCompile(
              source.c_str(),
              source.size(),
              name.c_str(),
              defines,
              nullptr,
              entryPoint.c_str(),
              GetShaderTarget(stage),
              compileFlags,
              0,
              &byteCodesBlob,
              &error)))
    {
        LOG::ERR("D3D12 Shader Compiling failed with...\n{}", (const char *)(error->GetBufferPointer()));
		byteCodesBlob->Release();
		byteCodesBlob = nullptr;
        return;
    }

    dxil.resize(byteCodesBlob->GetBufferSize());
	memcpy(dxil.data(), byteCodesBlob->GetBufferPointer(), dxil.size());

    Reflect();
}

void Shader::Reflect()
{
    ComPtr<ID3D12ShaderReflection> shaderReflection;;

    D3D12_SHADER_BYTECODE byteCodes = GetByteCodes();
	if (FAILED(D3DReflect(byteCodes.pShaderBytecode, byteCodes.BytecodeLength, IID_PPV_ARGS(&shaderReflection))))
    {
        LOG::ERR("Failed to reflect byte codes!");
        return;
    }

    SetupDescriptorRanges(shaderReflection);
}

void Shader::SetupDescriptorRanges(ComPtr<ID3D12ShaderReflection> shaderReflection)
{
    D3D12_SHADER_DESC desc{};
	Check(shaderReflection->GetDesc(&desc));

    for (uint32_t i = 0; i < desc.BoundResources; i++)
    {
        DescriptorRange range{};
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType{};
        D3D12_SHADER_INPUT_BIND_DESC inputDesc{};
		if (FAILED(shaderReflection->GetResourceBindingDesc(i, &inputDesc)))
        {
            break;
        }

        switch (inputDesc.Type)
		{
			case D3D_SIT_CBUFFER:
				if (!strcmp(inputDesc.Name, "push_constant") ||
				    !strcmp(inputDesc.Name, "PushConstant") ||
				    !strcmp(inputDesc.Name, "$Globals"))
				{
					D3D12_SHADER_BUFFER_DESC bufferDesc{};
					ID3D12ShaderReflectionConstantBuffer *constantBuffer = shaderReflection->GetConstantBufferByName(inputDesc.Name);
					Check(constantBuffer->GetDesc(&bufferDesc));
					pushConstants.size = bufferDesc.Size;
					pushConstants.biding = inputDesc.BindPoint;
					pushConstantIndex = i;
					continue;
				}
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				break;

			case D3D_SIT_TEXTURE:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				break;

			case D3D_SIT_SAMPLER:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				break;

            case D3D_SIT_UAV_RWTYPED:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				break;
			default:
				break;
		}
        range.Init(rangeType, inputDesc.BindCount, inputDesc.BindPoint, inputDesc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        descriptorRanges.emplace_back(range);
    }
}

}
}
