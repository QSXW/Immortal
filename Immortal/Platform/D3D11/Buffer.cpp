#include "Buffer.h"
#include "Device.h"
#include "RenderContext.h"

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
	if (type & Buffer::Type::Uniform)
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

Buffer::Buffer(const Buffer &other, const BindInfo &bindInfo) :
    Super{ bindInfo.type, bindInfo.size },
    device{}
{

}

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size },
    device{ device }
{
    __Create(data);
}

Buffer::Buffer(Device *device, const size_t size, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size },
	device{ device }
{
    __Create(nullptr);
}

Buffer::Buffer(Device *device, const size_t size, uint32_t binding) :
    Super{ Buffer::Type::Uniform, SLALIGN(size, 256) },
    device{ device }
{
    __Create(nullptr);
}

Buffer::~Buffer()
{

}

void Buffer::__Create(const void *data)
{
    D3D11_BUFFER_DESC desc = {
		.ByteWidth           = U32(size),
	    .Usage               = D3D11_USAGE_DEFAULT,
		.BindFlags           = CAST(type),
		.CPUAccessFlags      = 0,
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

void Buffer::Update(uint64_t updateSize, const void *data, uint64_t offset)
{
    THROWIF(updateSize > size, SError::OutOfBound);

    auto context = device->GetContext();

	context->UpdateSubresource(*this, 0, nullptr, data, 0, 0);
}

Buffer *Buffer::Bind(const BindInfo &bindInfo) const
{
    return new Buffer{ *this, bindInfo };
}

}
}
