#pragma once

#include "Common.h"
#include "Render/Descriptor.h"

namespace Immortal
{
namespace D3D12
{

struct CPUDescriptor : public SuperDescriptor, D3D12_CPU_DESCRIPTOR_HANDLE
{
public:
    CPUDescriptor() = default;

    explicit CPUDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &o) noexcept :
        D3D12_CPU_DESCRIPTOR_HANDLE{ o }
    {

    }

    CPUDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    CPUDescriptor &Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
        return *this;
    }

    void InitOffsetted(const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }

    static inline void InitOffsetted( D3D12_CPU_DESCRIPTOR_HANDLE &handle, const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        handle.ptr = SIZE_T(INT64(base.ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
    }
};

struct GPUDescriptor : public SuperDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE
{
public:
    GPUDescriptor() = default;

    explicit GPUDescriptor(const D3D12_GPU_DESCRIPTOR_HANDLE &o) noexcept :
        D3D12_GPU_DESCRIPTOR_HANDLE{ o }
    {

    }

    GPUDescriptor(const D3D12_GPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    GPUDescriptor& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
        return *this;
    }

    void InitOffsetted(const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }

    static inline void InitOffsetted( D3D12_GPU_DESCRIPTOR_HANDLE &handle, const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        handle.ptr = SIZE_T(INT64(base.ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
    }
};

struct Descriptor
{
    using Super = SuperDescriptor;

    enum class Type
    {
        ShaderResourceView,
        RenderTargetView,
        Sampler,
        DepthStencilView,
        UnorderedAccessView,
        ConstantBufferView
    };

    Descriptor(CPUDescriptor cpu, GPUDescriptor gpu) :
        cpu{ cpu },
        gpu{ gpu }
    {

    }

    Descriptor(D3D12_GPU_VIRTUAL_ADDRESS virtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL)
    {
        cpu.ptr = virtualAddress;
        gpu.ptr = virtualAddress;
    }

    Descriptor &Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        cpu.Offset(offsetInDescriptors, descriptorIncrementSize);
        gpu.Offset(offsetInDescriptors, descriptorIncrementSize);

        return *this;
    }

    CPUDescriptor cpu;
    GPUDescriptor gpu;
};

}
}
