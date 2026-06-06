#pragma once

#include "Common.h"
#include "Graphics/Buffer.h"
#include "Resource.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class DescriptorHeap;
class Buffer : public SuperBuffer, public Resource, public NonDispatchableHandle
{
public:
    using Super = SuperBuffer;

    struct VertexView : public D3D12_VERTEX_BUFFER_VIEW
    {
        using Primitive = D3D12_VERTEX_BUFFER_VIEW;

        VertexView()
        {
            Primitive::BufferLocation = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
            Primitive::SizeInBytes    = 0;
            Primitive::StrideInBytes  = 0;
        }

        VertexView(D3D12_GPU_VIRTUAL_ADDRESS location, UINT size, UINT stride)
        {
            Primitive::BufferLocation = location;
            Primitive::SizeInBytes    = size;
            Primitive::StrideInBytes  = stride;
        }
    };

    struct IndexView : public D3D12_INDEX_BUFFER_VIEW
    {
        using Primitive = D3D12_INDEX_BUFFER_VIEW;

        IndexView()
        {
            Primitive::BufferLocation = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
            Primitive::SizeInBytes    = 0;
            Primitive::Format         = DXGI_FORMAT_R32_UINT;
        }

        IndexView(D3D12_GPU_VIRTUAL_ADDRESS location, UINT size, DXGI_FORMAT format)
        {
            Primitive::BufferLocation = location;
            Primitive::SizeInBytes    = size;
            Primitive::Format         = format;
        }
    };

public:
	Buffer();

    Buffer(Device *device, Type type, const size_t size, Format format = Format::None, const void *data = nullptr);

    virtual ~Buffer() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual void Map(void **ppData, size_t size, uint64_t offset) override;

	virtual void Unmap() override;

    template <class T>
	HRESULT Map(T **data)
	{
		D3D12_RANGE range{ 0, GetSize() };
		return resource->Map(0, &range, (void **)data);
	}

    operator D3D12_GPU_VIRTUAL_ADDRESS() const
    {
        return virtualAddress;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc() const
    {
        return D3D12_CONSTANT_BUFFER_VIEW_DESC{ virtualAddress, uint32_t(GetSize()) };
    }

    Descriptor GetDescriptor() const
    {
        return descriptor;
    }

protected:
    void Construct(Format format);

protected:
    Descriptor descriptor;

    DescriptorHeap *descriptorHeap;
};

}
}
