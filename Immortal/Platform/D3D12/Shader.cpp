#include "Shader.h"

namespace Immortal
{
namespace D3D12
{

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
