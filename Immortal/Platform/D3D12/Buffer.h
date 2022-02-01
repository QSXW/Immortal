#pragma once

#include "Common.h"
#include "Render/Buffer.h"
#include "Resource.h"
#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Buffer : public SuperBuffer, public Resource
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
    Buffer(Device *device, const size_t size, const void *data, Type type);

    Buffer(Device *device, const size_t size, Type type);
    
    Buffer(Device *device, const size_t size, uint32_t binding);

    virtual ~Buffer() override;

    virtual void Update(uint32_t size, const void *data, uint32_t offset = 0) override;

    void Map(void **data)
    {
        D3D12_RANGE range{ 0, size };
        resource->Map(0, &range, data);
    }

    void Unmap()
    {
        D3D12_RANGE range{ 0, size };
        resource->Unmap(0, &range);
    }

    operator D3D12_GPU_VIRTUAL_ADDRESS() const
    {
        return virtualAddress;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc() const
    {
        return D3D12_CONSTANT_BUFFER_VIEW_DESC{ virtualAddress, size };
    }

    uint32_t Binding() const
    {
        return binding;
    }

private:
    void InternelCreate(const Device *device);

private:
    uint32_t binding = 0;
};

}
}
