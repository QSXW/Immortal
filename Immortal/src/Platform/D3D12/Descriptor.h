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

    Descriptor& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
        return *this;
    }
};

}
}
