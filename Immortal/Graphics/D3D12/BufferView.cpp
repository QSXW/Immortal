#include "BufferView.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

BufferView::BufferView(Device *device, Buffer *buffer, Format format) :
    NonDispatchableHandle{ device },
    descriptor{},
    descriptorHeap{}
{
	Create(buffer, format, format.GetTexelSize());
}

BufferView::BufferView(Device *device, Buffer *buffer, uint32_t byteStride) :
    NonDispatchableHandle{ device },
    descriptor{},
    descriptorHeap{}
{
	Create(buffer, Format::None, byteStride);
}

void BufferView::Create(Buffer *buffer, Format format, uint32_t byteStride)
{
    descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &descriptorHeap, 2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
        .Format                  = format,
        .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Buffer                  = {
            .FirstElement        = 0,
            .NumElements         = UINT(buffer->GetSize() / byteStride),
            .StructureByteStride = format != Format::None ? 0 : UINT(byteStride),
            .Flags               = D3D12_BUFFER_SRV_FLAG_NONE,
        }
    };

    device->CreateShaderResourceView(*buffer, &srvDesc, descriptor[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]);

    D3D12_UNORDERED_ACCESS_VIEW_DESC desc {
        .Format        = format,
        .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
        .Buffer = {
            .FirstElement         = 0,
            .NumElements          = UINT(buffer->GetSize() / byteStride),
            .StructureByteStride  = format != Format::None ? 0 : UINT(byteStride),
            .CounterOffsetInBytes = 0,
            .Flags                = D3D12_BUFFER_UAV_FLAG_NONE,
        }
    };
    device->CreateUnorderedAccessView(*buffer, nullptr, &desc, descriptor[D3D12_DESCRIPTOR_RANGE_TYPE_UAV]);
}

BufferView::~BufferView()
{
	if (device && descriptorHeap)
	{
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeap, descriptor);
	}
}

}
}
