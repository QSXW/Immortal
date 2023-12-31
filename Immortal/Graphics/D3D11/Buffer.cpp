#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

UINT CAST(Buffer::Type type)
{
	UINT32 flags = 0;
    if (type & Buffer::Type::Vertex)
    {
		flags |= D3D11_BIND_VERTEX_BUFFER;
    }
	if (type & Buffer::Type::Index)
	{
		flags |= D3D11_BIND_INDEX_BUFFER;
	}
	if (type & Buffer::Type::ConstantBuffer)
	{
		flags |= D3D11_BIND_CONSTANT_BUFFER;
	}
	if (type & Buffer::Type::PushConstant)
	{
		flags |= D3D11_BIND_CONSTANT_BUFFER;
	}
	if (type & Buffer::Type::Storage)
	{
		flags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	if (type & Buffer::Type::VideoDecoder)
	{
		flags |= D3D11_BIND_DECODER;
	}
	if (type & Buffer::Type::VideoEncoder)
	{
		flags |= D3D11_BIND_VIDEO_ENCODER;
	}

	return flags;
}

uint32_t GetAccessFlags(BufferType type, uint32_t *pUsage, uint32_t *pMapFlags)
{
	uint32_t flags = 0;
	if (type & BufferType::TransferSource)
	{
		flags      |= D3D11_CPU_ACCESS_WRITE;
		*pMapFlags = D3D11_MAP_WRITE;
		*pUsage = D3D11_USAGE_STAGING;
	}
	if (type & BufferType::TransferDestination)
	{
		flags      |= D3D11_CPU_ACCESS_READ;
		*pMapFlags = D3D11_MAP_READ;
		*pUsage = D3D11_USAGE_STAGING;
	}

	return flags;
}

Buffer::Buffer():
	Super{},
    NonDispatchableHandle{},
    mapFlags{}
{

}

Buffer::Buffer(Device *device, const size_t size, Type type, const void *data) :
    Super{ type, type == Type::ConstantBuffer ? SLALIGN(size, 256) : size },
    NonDispatchableHandle{ device },
    mapFlags{}
{
	ConstructHandle(data);
}

Buffer::~Buffer()
{
	handle.Reset();
}

void Buffer::ConstructHandle(const void *data)
{
	auto type = GetType();
	uint32_t usage = D3D11_USAGE_DEFAULT;
	uint32_t accessFlags = GetAccessFlags(type, & usage, &mapFlags);
    D3D11_BUFFER_DESC desc = {
		.ByteWidth           = uint32_t(GetSize()),
	    .Usage               = D3D11_USAGE(usage),
		.BindFlags           = CAST(type),
		.CPUAccessFlags      = accessFlags,
		.MiscFlags           = 0,
		.StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA subresource = {
        .pSysMem          = data,
        .SysMemPitch      = 0,
        .SysMemSlicePitch = 0,
    };

    Check(device->CreateBuffer(&desc, data ? &subresource : nullptr, &handle));
}

Anonymous Buffer::GetBackendHandle() const
{
	return (void *)handle.Get();
}

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	device->Map(handle.Get(), 0, (D3D11_MAP)mapFlags, 0, &mappedSubresource);
	*ppData = (uint8_t *)mappedSubresource.pData + offset;
}

void Buffer::Unmap()
{
	device->Unmap(handle.Get(), 0);
}

}
}
