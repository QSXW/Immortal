#pragma once
#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class DescriptorPool;

struct Descriptor : D3D12_CPU_DESCRIPTOR_HANDLE
{
public:
    Descriptor() = default;

    explicit Descriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &o) noexcept :
        D3D12_CPU_DESCRIPTOR_HANDLE{ o }
    {

    }

    Descriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    Descriptor& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
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

}
}
