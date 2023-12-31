#pragma once
#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class RootSignature
{
public:
    enum class Flag
    {
        None                           = D3D12_ROOT_SIGNATURE_FLAG_NONE,
        AllowInputAssemblerInputLayout = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };

	struct Description : public D3D12_VERSIONED_ROOT_SIGNATURE_DESC
	{
        using Primitive = D3D12_VERSIONED_ROOT_SIGNATURE_DESC;

        Description() noexcept
        {
            Init_1_1(0, nullptr, 0, nullptr, Flag::None);
        }

        explicit Description(const Primitive &p) noexcept :
            Primitive{ p }
        {
        
        }

        explicit Description(const D3D12_ROOT_SIGNATURE_DESC &o) noexcept
        {
            Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            Desc_1_0 = o;
        }

        explicit Description(const D3D12_ROOT_SIGNATURE_DESC1 &o) noexcept
        {
            Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            Desc_1_1 = o;
        }

        Description(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init_1_0(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);
        }

        Description(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER1 * pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC * pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init_1_1(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);
        }

        void Init_1_0(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init_1_0(*this, numParameters, pParameters, numStaticSamplers, pStaticSamplers, D3D12_ROOT_SIGNATURE_FLAGS(flags));
        }

        static inline void Init_1_0(
            Primitive &desc,
            UINT numParameters,
            const D3D12_ROOT_PARAMETER *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
        {
            desc.Version                    = D3D_ROOT_SIGNATURE_VERSION_1_0;
            desc.Desc_1_0.NumParameters     = numParameters;
            desc.Desc_1_0.pParameters       = pParameters;
            desc.Desc_1_0.NumStaticSamplers = numStaticSamplers;
            desc.Desc_1_0.pStaticSamplers   = pStaticSamplers;
            desc.Desc_1_0.Flags = flags;
        }

        void Init_1_1(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER1 *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init_1_1(*this, numParameters, pParameters, numStaticSamplers, pStaticSamplers, D3D12_ROOT_SIGNATURE_FLAGS(flags));
        }

        static inline void Init_1_1(
            Primitive &desc,
            UINT numParameters,
            const D3D12_ROOT_PARAMETER1 *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
        {
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            desc.Desc_1_1.NumParameters     = numParameters;
            desc.Desc_1_1.pParameters       = pParameters;
            desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
            desc.Desc_1_1.pStaticSamplers   = pStaticSamplers;
            desc.Desc_1_1.Flags             = flags;
        }
	};

public:
    ~RootSignature()
    {

    }

public:
    using Primitive = ID3D12RootSignature;
    D3D12_OPERATOR_HANDLE()
};

struct RootDescriptorTable : public D3D12_ROOT_DESCRIPTOR_TABLE1
{
    using Primitive = D3D12_ROOT_DESCRIPTOR_TABLE1;

    RootDescriptorTable()
    {

    }

    explicit RootDescriptorTable(const Primitive &p) noexcept :
        Primitive{ p }
    {
    
    }

    RootDescriptorTable(
        UINT numDescriptorRanges,
        const DescriptorRange *pDescriptorRanges) noexcept
    {
        Init(numDescriptorRanges, pDescriptorRanges);
    }

    inline void Init(
        UINT numDescriptorRanges,
        const DescriptorRange *pDescriptorRanges) noexcept
    {
        Init(*this, numDescriptorRanges, pDescriptorRanges);
    }

    static inline void Init(
        Primitive &rootDescriptorTable,
        UINT numDescriptorRanges,
        const DescriptorRange *pDescriptorRanges) noexcept
    {
        rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
        rootDescriptorTable.pDescriptorRanges   = pDescriptorRanges;
    }
};

struct RootDescriptor : public D3D12_ROOT_DESCRIPTOR1
{
    using Primitive = D3D12_ROOT_DESCRIPTOR1;

    RootDescriptor()
    {

    }

    explicit RootDescriptor(const Primitive &p) noexcept :
        Primitive{ p }
    {
    
    }

    RootDescriptor(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
    {
        Init(shaderRegister, registerSpace, flags);
    }

    inline void Init(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
    {
        Init(*this, shaderRegister, registerSpace, flags);
    }

    static inline void Init(
        Primitive &table,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept
    {
        table.ShaderRegister = shaderRegister;
        table.RegisterSpace  = registerSpace;
        table.Flags          = flags;
    }
};

struct RootConstants : public D3D12_ROOT_CONSTANTS
{
    using Primitive = D3D12_ROOT_CONSTANTS;

    RootConstants()
    {

    }

    explicit RootConstants(const Primitive &p) noexcept :
        D3D12_ROOT_CONSTANTS{ p }
    {
    
    }

    RootConstants(
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0) noexcept
    {
        Init(num32BitValues, shaderRegister, registerSpace);
    }

    inline void Init(
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0) noexcept
    {
        Init(*this, num32BitValues, shaderRegister, registerSpace);
    }

    static inline void Init(
        Primitive &rootConstants,
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0) noexcept
    {
        rootConstants.Num32BitValues = num32BitValues;
        rootConstants.ShaderRegister = shaderRegister;
        rootConstants.RegisterSpace = registerSpace;
    }
};

struct RootParameter : public D3D12_ROOT_PARAMETER1
{
    using Primitive = D3D12_ROOT_PARAMETER1;

    RootParameter()
    {

    }

    explicit RootParameter(const Primitive &p) noexcept :
        Primitive{ p }
    {
    
    }

    static inline void InitAsDescriptorTable(
        Primitive &rootParam,
        UINT numDescriptorRanges,
        const DescriptorRange *pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        rootParam.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam.ShaderVisibility = visibility;
        RootDescriptorTable::Init(rootParam.DescriptorTable, numDescriptorRanges, pDescriptorRanges);
    }

    static inline void InitAsConstants(
        Primitive &rootParam,
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        rootParam.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParam.ShaderVisibility = visibility;
        RootConstants::Init(rootParam.Constants, num32BitValues, shaderRegister, registerSpace);
    }

    static inline void InitAsConstantBufferView(
        D3D12_ROOT_PARAMETER1 &rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        rootParam.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParam.ShaderVisibility = visibility;
        RootDescriptor::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
    }

    static inline void InitAsShaderResourceView(
        D3D12_ROOT_PARAMETER1 &rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        rootParam.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParam.ShaderVisibility = visibility;
        RootDescriptor::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
    }

    static inline void InitAsUnorderedAccessView(
        D3D12_ROOT_PARAMETER1 &rootParam,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        rootParam.ShaderVisibility = visibility;
        RootDescriptor::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
    }

    inline void InitAsDescriptorTable(
        UINT numDescriptorRanges,
        const DescriptorRange *pDescriptorRanges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        InitAsDescriptorTable(*this, numDescriptorRanges, pDescriptorRanges, visibility);
    }

    inline void InitAsConstants(
        UINT num32BitValues,
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        InitAsConstants(*this, num32BitValues, shaderRegister, registerSpace, visibility);
    }

    inline void InitAsConstantBufferView(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        InitAsConstantBufferView(*this, shaderRegister, registerSpace, flags, visibility);
    }

    inline void InitAsShaderResourceView(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        InitAsShaderResourceView(*this, shaderRegister, registerSpace, flags, visibility);
    }

    inline void InitAsUnorderedAccessView(
        UINT shaderRegister,
        UINT registerSpace = 0,
        D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        InitAsUnorderedAccessView(*this, shaderRegister, registerSpace, flags, visibility);
    }
};

}
}
