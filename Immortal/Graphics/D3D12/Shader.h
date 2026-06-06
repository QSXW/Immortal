#pragma once

#include "Common.h"
#include "Graphics/Shader.h"

#include <array>
#include <d3d12shader.h>

namespace Immortal
{
namespace D3D12
{

struct PushConstants
{
	uint8_t size;
	uint8_t biding;
};

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

public:
	Shader(const std::string &name, Stage stage, const std::string &source, const std::string &entryPoint, const ShaderMacro *pMacro = nullptr, uint32_t numMacro = 0);

    Shader(Stage stage, ShaderBinaryType type, const void *binary, uint32_t size);

    virtual ~Shader() override;

    void LoadByteCodes(const std::string &source, const std::string &name, ShaderStage stage, const std::string &entryPoint, const ShaderMacro *pMacro = nullptr, uint32_t numMacro = 0);

public:
    D3D12_SHADER_BYTECODE GetByteCodes() const
    {
		return D3D12_SHADER_BYTECODE{
		    .pShaderBytecode = dxil.data(),
		    .BytecodeLength  = dxil.size(),
        };
    }

    const std::vector<DescriptorRange> &GetDescriptorRanges() const
    {
        return descriptorRanges;
    }

    const PushConstants &GetPushConstants() const
    {
		return pushConstants;
    }

    D3D12_SHADER_VISIBILITY GetVisibility() const
    {
		return visibility;
    }

    uint32_t GetPushConstantIndex() const
    {
		return pushConstantIndex;
    }

protected:
    void Reflect();

    void SetupDescriptorRanges(ComPtr<ID3D12ShaderReflection> reflector);

protected:
	std::vector<uint8_t> dxil;

    std::vector<DescriptorRange> descriptorRanges;

    D3D12_SHADER_VISIBILITY visibility;

    PushConstants pushConstants;

    uint32_t pushConstantIndex;
};

}
}
