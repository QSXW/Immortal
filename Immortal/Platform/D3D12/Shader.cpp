#include "Shader.h"
#include "Framework/Utils.h"

namespace Immortal
{
namespace D3D12
{

Shader::Shader(const std::string &filepath, Type type) :
    Super{ type }
{
    constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ComPtr<ID3DBlob> error;

    std::wstring wfilepath = Utils::s2ws(filepath) + std::wstring{ L".hlsl" };
    if (type == Type::Graphics)
    {
        InternalCheck(
            D3DCompileFromFile(
                wfilepath.c_str(),
                nullptr,
                nullptr,
                "VSMain",
                "vs_5_1",
                compileFlags,
                0,
                &handles[VertexShaderPos],
                &error
            ),
            &error,
            &handles[VertexShaderPos]
            );

        byteCodes[VertexShaderPos] = ShaderByteCode{ handles[VertexShaderPos] };

        InternalCheck(
            D3DCompileFromFile(
                wfilepath.c_str(),
                nullptr,
                nullptr,
                "PSMain",
                "ps_5_1",
                compileFlags,
                0,
                &handles[PixelShaderPos],
                &error
            ),
            &error,
            &handles[PixelShaderPos]
        );

        byteCodes[PixelShaderPos] = ShaderByteCode{ handles[PixelShaderPos] };
    }

    Reflect();
}

Shader::~Shader()
{
    for (auto &handle : handles)
    {
        IfNotNullThenRelease(handle);
    }
}

void Shader::InternalCheck(HRESULT result, ID3DBlob **error, ID3DBlob **toBeReleased)
{
    if (FAILED(result) || !*toBeReleased)
    {
        if (*error)
        {
            LOG::ERR("D3D12 Shader Compiling failed with...\n{0}", rcast<char *>((*error)->GetBufferPointer()));
        }

        if (*toBeReleased)
        {
            (*toBeReleased)->Release();
            *toBeReleased = nullptr;
        }

        THROWIF(true, "Failed to compile shader source");
    }
}

void Shader::Reflect()
{
    ComPtr<ID3D12ShaderReflection> reflector = NULL;;

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

        D3D12_SHADER_DESC desc{};
        Check(reflector->GetDesc(&desc));
        SetupDescriptorRanges(desc, IsGraphics() ?
            (i == VertexShaderPos ? D3D12_SHADER_VISIBILITY_VERTEX : D3D12_SHADER_VISIBILITY_PIXEL) : D3D12_SHADER_VISIBILITY_ALL);
    }
}

void Shader::SetupDescriptorRanges(const D3D12_SHADER_DESC &desc, D3D12_SHADER_VISIBILITY visibility)
{
    if (desc.ConstantBuffers)
    {
        DescriptorRange range{};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, desc.ConstantBuffers, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        descriptorRanges.emplace_back(std::pair(range, visibility));
    }
    if (desc.TextureNormalInstructions)
    {
        DescriptorRange range{};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, desc.TextureNormalInstructions, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        descriptorRanges.emplace_back(std::pair(range, visibility));
    }
}

} 
}
