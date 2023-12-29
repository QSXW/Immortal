#pragma once

#include "Common.h"
#include "Render/Descriptor.h"

namespace Immortal
{
namespace D3D11
{

using SRV = ID3D11ShaderResourceView;
using RTV = ID3D11RenderTargetView;
using DSV = ID3D11DepthStencilView;
using UAV = ID3D11UnorderedAccessView;

template <class T>
struct Descriptor
{
    using Primitive = T;
    D3D11_OPERATOR_HANDLE()
    D3D11_SWAPPABLE(Descriptor)

    Descriptor() :
        handle{}
    {

    }

    ~Descriptor()
    {
        handle.Reset();
    }

    void Swap(Descriptor &other)
    {
        std::swap(handle, other.handle);
    }

    operator uint64_t() const
    {
        return (uint64_t)handle.Get();
    }
};

}
}
