#pragma once

#include "D3D12Common.h"
#include "Render/Shader.h"
#include "Framework/Utils.h"

#include <array>
#include <d3dcompiler.h>

namespace Immortal
{
namespace D3D12
{

struct ShaderByteCode : public D3D12_SHADER_BYTECODE
{
    ShaderByteCode()
    {
        pShaderBytecode = nullptr;
        BytecodeLength  = 0;
    };

    explicit ShaderByteCode(const D3D12_SHADER_BYTECODE &o) noexcept :
        D3D12_SHADER_BYTECODE(o)
    {}

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

#ifdef  SLDEBUG
    static inline constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    static inline constexpr UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    enum
    {
        VertexShaderPos      = 0,
        PixelShaderPos       = 1,
        ComputeShaderPos     = 0,
        MaxHandleCount       = 2
    };

public:
    Shader(const std::string &filepath, Type type = Type::Graphics)
    {
        ComPtr<ID3DBlob> errorMsg{ nullptr };

        std::wstring wfilepath = Utils::s2ws(filepath) + std::wstring{ L".hlsl" };
        if (type == Type::Graphics)
        {
            InternalCheck(D3DCompileFromFile(
                    wfilepath.c_str(),
                    nullptr,
                    nullptr,
                    "VSMain",
                    "vs_5_0",
                    compileFlags,
                    0,
                    &handles[VertexShaderPos],
                    &errorMsg
                    ),
                &errorMsg,
                &handles[VertexShaderPos]
                );

            byteCodes[VertexShaderPos] = ShaderByteCode{ handles[VertexShaderPos] };

            InternalCheck(
                D3DCompileFromFile(
                    wfilepath.c_str(),
                    nullptr,
                    nullptr,
                    "PSMain",
                    "ps_5_0",
                    compileFlags,
                    0,
                    &handles[PixelShaderPos],
                    &errorMsg
                    ),
                &errorMsg,
                &handles[PixelShaderPos]
                );

            byteCodes[PixelShaderPos] = ShaderByteCode{ handles[PixelShaderPos] };
        }

        if (errorMsg)
        {
            errorMsg->Release();
        }
    }

    std::array<ShaderByteCode, MaxHandleCount> ByteCodes()
    {
        return byteCodes;
    }

    virtual ~Shader()
    {
        for (auto &handle : handles)
        {
            IfNotNullThenRelease(handle);
        }
    }

private:
    void InternalCheck(HRESULT result, ID3DBlob **errorMsg, ID3DBlob **toBeReleased)
    {
         if ( FAILED(result) )
         {
             if (*errorMsg)
             {
                 LOG::ERR("D3D12 Shader Compiling failed with...\n{0}", rcast<char *>((*errorMsg)->GetBufferPointer()));
                 (*errorMsg)->Release();
                 *errorMsg = nullptr;
             }
         
             if (*toBeReleased)
             {
                 (*toBeReleased)->Release();
                 *toBeReleased = nullptr;
             }
         }
    }

private:
    std::array<ID3DBlob*, MaxHandleCount> handles{ nullptr };

    std::array<ShaderByteCode, MaxHandleCount> byteCodes;
};

}
}