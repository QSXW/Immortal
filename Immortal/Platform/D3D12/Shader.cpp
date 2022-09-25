#include "Shader.h"
#include "Framework/Utils.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace D3D12
{

Shader::Shader(const std::string &filepath, Type type) :
    Super{ type },
    handles{},
    pushConstants{}
{
	std::string path = filepath + ".hlsl";
	auto source = FileSystem::ReadString(path);
	LoadByteCodes(source, path, type);
}

Shader::Shader(const std::string &name, const std::string &source, Type type) :
    Super{ type },
    handles{},
    pushConstants{}
{
	LoadByteCodes(source, name);
}

Shader::~Shader()
{
    for (auto &handle : handles)
    {
        if (handle)
        {
            handle->Release();
        }
    }
}

void Shader::LoadByteCodes(const std::string &source, const std::string &name, Type type)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> error;
	const char *target = "vs_5_1";
	const char *entryPoint = "VSMain";

	if (type == Type::Compute)
	{
		target = "cs_5_1";
		entryPoint = "main";
	}

    D3D_SHADER_MACRO defines[] = {
	    { .Name = "__D3D12__", .Definition = NULL },
	    { .Name = NULL,        .Definition = NULL },
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
	        &handles[VertexShaderPos],
	        &error),
	    &error,
	    &handles[VertexShaderPos]);

	byteCodes[VertexShaderPos] = ShaderByteCode{handles[VertexShaderPos]};

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
		        "ps_5_1",
		        compileFlags,
		        0,
		        &handles[PixelShaderPos],
		        &error),
		    &error,
		    &handles[PixelShaderPos]);

		byteCodes[PixelShaderPos] = ShaderByteCode{handles[PixelShaderPos]};
	}

	__Reflect();
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

void Shader::__Reflect()
{
    ComPtr<ID3D12ShaderReflection> reflector = NULL;;

    uint32_t baseRegisters[8] = { 0 };

    for (size_t i = 0; i < byteCodes.size(); i++)
    {
        if (!byteCodes[i].pShaderBytecode)
        {
            break;
        }
        D3DReflect(
            byteCodes[i].pShaderBytecode,
            byteCodes[i].BytecodeLength,
            IID_PPV_ARGS(&reflector)
        );

		__SetupDescriptorRanges(reflector, IsGraphics() ?
            (i == VertexShaderPos ? D3D12_SHADER_VISIBILITY_VERTEX : D3D12_SHADER_VISIBILITY_PIXEL) : D3D12_SHADER_VISIBILITY_ALL, baseRegisters);
    }
}

void Shader::__SetupDescriptorRanges(ComPtr<ID3D12ShaderReflection> reflector, D3D12_SHADER_VISIBILITY visibility, uint32_t *refBaseRegisters)
{
    DescriptorRange range{};
	D3D12_SHADER_DESC desc{};
	Check(reflector->GetDesc(&desc));

    if (desc.ConstantBuffers)
    {
		int i = 0;
        for (; i < desc.ConstantBuffers; i++)
        {
			D3D12_SHADER_BUFFER_DESC bufferDesc;
			auto constantBuffer = reflector->GetConstantBufferByIndex(i);
			Check(constantBuffer->GetDesc(&bufferDesc));
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			Check(reflector->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc));
            if (!strcmp(bufferDesc.Name, "push_constant"))
            {
				pushConstants.size = bufferDesc.Size;
				pushConstants.biding = bindDesc.BindPoint;
                if (i > 0)
                {
					range.Init(
					    D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
					    i,
					    0, 0,
					    D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
					descriptorRanges.emplace_back(std::pair(range, visibility));
                }
				break;
            }
        }
        if (i == desc.ConstantBuffers)
        {
			range.Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				desc.ConstantBuffers,
				0, 0,
				D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
			descriptorRanges.emplace_back(std::pair(range, visibility));
        }
        else if (i + 1 < desc.ConstantBuffers)
        {
			range.Init(
			    D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			    desc.ConstantBuffers - i,
			    i + 1, 0,
			    D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
			descriptorRanges.emplace_back(std::pair(range, visibility));
        }
    }
    if (desc.TextureNormalInstructions)
    {
        range.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            desc.TextureNormalInstructions,
            refBaseRegisters[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]++, 0,
            D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
        );
        descriptorRanges.emplace_back(std::pair(range, visibility));
    }
    if (desc.BoundResources && IsType(Type::Compute))
    {
        range.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            desc.BoundResources - desc.ConstantBuffers,
            0, 0,
            D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
        );
        descriptorRanges.emplace_back(std::pair(range, visibility));
    }
}

}
}
