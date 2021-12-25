#pragma once

#include "Common.h"
#include "Render/Buffer.h"
#include "Device.h"
#include "Resource.h"

namespace Immortal
{
namespace D3D12
{

class Buffer : public SuperBuffer, public Resource
{
public:
    using Super = SuperBuffer;

    using HandleType = ID3D12Resource*;

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
    
    virtual ~Buffer() override;

    operator const HandleType() const
    {
        return resource;
    }

    virtual void Update(uint32_t size, const void *data) override;

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
        return gpuVirtualAddress;
    }

private:
    void InternelCreate(const Device *device);
};

}
}
