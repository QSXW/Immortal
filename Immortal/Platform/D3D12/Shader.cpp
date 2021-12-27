#include "Shader.h"

namespace Immortal
{
namespace D3D12
{

void Shader::Reflect()
{
    ID3D12ShaderReflection* reflector = NULL;;

    auto &byteCode = byteCodes[VertexShaderPos];
    D3DReflect(
        byteCode.pShaderBytecode,
        byteCode.BytecodeLength,
        IID_PPV_ARGS(&reflector)
    );

    D3D12_SHADER_DESC desc{};
    Check(reflector->GetDesc(&desc));

    if (desc.ConstantBuffers)
    {
        DescriptorRange range{};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, desc.ConstantBuffers, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        descriptorRanges.emplace_back(range);
    }
    if (desc.TextureLoadInstructions)
    {
        DescriptorRange range{};
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, desc.TextureLoadInstructions, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        descriptorRanges.emplace_back(range);
    }
}

} 
}
