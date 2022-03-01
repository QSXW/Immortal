#pragma once

#include "Common.h"
#include "Render/Shader.h"

#include <array>
#include <d3dcompiler.h>
#include <d3d12shader.h>

namespace Immortal
{
namespace D3D12
{

struct ShaderByteCode : public D3D12_SHADER_BYTECODE
{
    using Primitive = D3D12_SHADER_BYTECODE;

    ShaderByteCode()
    {
        pShaderBytecode = nullptr;
        BytecodeLength  = 0;
    };

    explicit ShaderByteCode(const Primitive &p) noexcept :
        Primitive{ p }
    {
    
    }

    ShaderByteCode(ID3DBlob *pShaderBlob) noexcept
    {
        pShaderBytecode = pShaderBlob->GetBufferPointer();
        BytecodeLength  = pShaderBlob->GetBufferSize();
    }

    ShaderByteCode(const void *_pShaderBytecode, SIZE_T bytecodeLength ) noexcept
    {
        pShaderBytecode = _pShaderBytecode;
        BytecodeLength  = bytecodeLength;
    }
};

class Shader : public SuperShader
{
public:
    using Super = SuperShader;

    enum
    {
        VertexShaderPos      = 0,
        PixelShaderPos       = 1,
        ComputeShaderPos     = 0,
        MaxHandleCount       = 2
    };

public:
    Shader(const std::string &filepath, Type type = Type::Graphics);

    virtual ~Shader();

    const std::array<ShaderByteCode, MaxHandleCount> &ByteCodes() const
    {
        return byteCodes;
    }

    const std::vector<std::pair<DescriptorRange, D3D12_SHADER_VISIBILITY>> &DescriptorRanges() const
    {
        return descriptorRanges;
    }

private:
    void InternalCheck(HRESULT result, ID3DBlob **errorMsg, ID3DBlob **toBeReleased);

    void Reflect();

    void SetupDescriptorRanges(const D3D12_SHADER_DESC &desc, D3D12_SHADER_VISIBILITY visibility);

private:
    std::array<ID3DBlob*, MaxHandleCount> handles{ nullptr };

    std::array<ShaderByteCode, MaxHandleCount> byteCodes;

    std::vector<std::pair<DescriptorRange, D3D12_SHADER_VISIBILITY>> descriptorRanges;
};

}
}
