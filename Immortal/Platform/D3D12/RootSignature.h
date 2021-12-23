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

	struct Description : public D3D12_ROOT_SIGNATURE_DESC
	{
        using Primitive = D3D12_ROOT_SIGNATURE_DESC;

        Description() noexcept
        {
            Init(0, nullptr, 0, nullptr, Flag::None);
        }

        explicit Description(const Primitive &o) noexcept :
            Primitive{ o }
        {
        
        }

        Description(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER * pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC * pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);
        }

        inline void Init(
            UINT numParameters,
            const D3D12_ROOT_PARAMETER * pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC * pStaticSamplers = nullptr,
            Flag flags = Flag::None) noexcept
        {
            Init(*this, numParameters, pParameters, numStaticSamplers, pStaticSamplers, D3D12_ROOT_SIGNATURE_FLAGS(flags));
        }

        static inline void Init(
            Primitive &desc,
            UINT numParameters,
            const D3D12_ROOT_PARAMETER *pParameters,
            UINT numStaticSamplers = 0,
            const D3D12_STATIC_SAMPLER_DESC *pStaticSamplers = nullptr,
            D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
        {
            desc.NumParameters     = numParameters;
            desc.pParameters       = pParameters;
            desc.NumStaticSamplers = numStaticSamplers;
            desc.pStaticSamplers   = pStaticSamplers;
            desc.Flags             = flags;
        }
	};

    operator ID3D12RootSignature*()
    {
        return handle.Get();
    }

    ID3D12RootSignature **operator &()
    {
        return handle.GetAddressOf();
    }

private:
    ComPtr<ID3D12RootSignature> handle;
};

}
}
