#pragma once

#include "Common.h"
#include "Graphics/Descriptor.h"

namespace Immortal
{
namespace D3D12
{

struct DescriptorHandle : public D3D12_CPU_DESCRIPTOR_HANDLE
{
public:
	DescriptorHandle()
    {
		ptr = 0;
    }

    DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE &o) noexcept :
	    D3D12_CPU_DESCRIPTOR_HANDLE{ o }
    {

    }

    DescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE &o) noexcept :
	    D3D12_CPU_DESCRIPTOR_HANDLE{ (D3D12_CPU_DESCRIPTOR_HANDLE &)o }
	{

	}

    DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    DescriptorHandle &Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
        return *this;
    }

    DescriptorHandle &Offset(UINT descriptorIncrementSize) noexcept
    {
        return Offset(1, descriptorIncrementSize);
    }

    void InitOffsetted(const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }

    static inline void InitOffsetted(D3D12_CPU_DESCRIPTOR_HANDLE &handle, const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
    {
        handle.ptr = SIZE_T(INT64(base.ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
    }

    operator D3D12_CPU_DESCRIPTOR_HANDLE() const
	{
		return (D3D12_CPU_DESCRIPTOR_HANDLE &)this->ptr;
	}

	operator D3D12_GPU_DESCRIPTOR_HANDLE() const
	{
		return (D3D12_GPU_DESCRIPTOR_HANDLE &)this->ptr;
	}
};

class Descriptor : public DescriptorHandle
{
public:
    using Super = DescriptorHandle;

    Descriptor() :
        Super{},
	    incrementSize{}
    {

    }

    Descriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &handle, uint32_t descriptorIncrementSize) :
        Super{ handle },
        incrementSize{ descriptorIncrementSize }
    {

    }

    Descriptor(const D3D12_GPU_DESCRIPTOR_HANDLE &handle, uint32_t descriptorIncrementSize) :
	    Super{ (const D3D12_CPU_DESCRIPTOR_HANDLE&)handle },
	    incrementSize{ descriptorIncrementSize }
	{

	}

    DescriptorHandle operator[](uint32_t index)
    {
		DescriptorHandle handle{ *this };
		return handle.Offset(index, incrementSize);
    }

    uint32_t GetIncrementSize() const
    {
		return incrementSize;
    }

protected:
	uint32_t incrementSize;
};

struct ShaderVisibleDescriptor
{
	Descriptor descriptor;

	DescriptorHandle shaderVisibleDescriptor;

    operator D3D12_GPU_DESCRIPTOR_HANDLE() const
    {
		return shaderVisibleDescriptor;
    }

    operator D3D12_CPU_DESCRIPTOR_HANDLE() const
    {
		return descriptor;
    }

    operator const Descriptor&() const
    {
		return descriptor;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE operator[](size_t index) const
    {
        DescriptorHandle handle{ shaderVisibleDescriptor };
		return handle.Offset(index, descriptor.GetIncrementSize());
    }

    ShaderVisibleDescriptor &Offset(uint32_t index)
    {
		descriptor.Offset(index, descriptor.GetIncrementSize());
		shaderVisibleDescriptor.Offset(index, descriptor.GetIncrementSize());
		return *this;
    }
};

}
}
