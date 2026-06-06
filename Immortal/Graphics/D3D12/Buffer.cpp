#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Buffer::Buffer() :
    Super{},
    NonDispatchableHandle{},
    descriptorHeap{}
{

}

Buffer::Buffer(Device *device, Type type, size_t size, MemoryType memoryType, uint32_t byteStride) :
    Super{ type, size },
    NonDispatchableHandle{ device },
    descriptorHeap{}
{
	Construct(memoryType, byteStride);
}

Buffer::~Buffer()
{
    if (GetType() & Type::ConstantBuffer)
    {
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeap, descriptor);
    }
}

void Buffer::Construct(MemoryType memoryType, uint32_t byteStride)
{
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

    D3D12_HEAP_PROPERTIES heapProperties = {
        .Type                 = D3D12_HEAP_TYPE_DEFAULT,
        .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask     = 1,
        .VisibleNodeMask      = 1,
    };

	const auto &type = GetType();
    bool isConstantBuffer = type & Type::ConstantBuffer;

    D3D12_RESOURCE_DESC desc = {
        .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment        = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, // Zero is effectively 64KB also
        .Width            = isConstantBuffer ? SLALIGN(GetSize(), 256): GetSize(),
        .Height           = 1,
        .DepthOrArraySize = 1,
        .MipLevels        = 1,
        .Format           = DXGI_FORMAT_UNKNOWN,
        .SampleDesc       = { .Count = 1, .Quality = 0 },
        .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags            = D3D12_RESOURCE_FLAG_NONE,
    };

    if (type & Type::Storage)
    {
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    if (isConstantBuffer)
    {
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		state = D3D12_RESOURCE_STATE_COMMON;
    }
	
    if (memoryType == MemoryType::Host)
	{
		if (type & Type::TransferDestination)
		{
			heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else
		{
			heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
	}

	if (type & Type::Scratch)
	{
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
    else if (type & Type::AccelerationStructure)
    {
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    DX_CHECK(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        state,
        nullptr,
        &resource
    ));

    virtualAddress = resource->GetGPUVirtualAddress();

    if (type & (Type::ConstantBuffer | Type::Storage))
    {
		descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &descriptorHeap, D3D12_DESCRIPTOR_RANGE_TYPE_CBV + 1);

        if (isConstantBuffer)
        {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {
			    .BufferLocation = GetGPUVirtualAddress(),
			    .SizeInBytes    = UINT(desc.Width)
            };
			device->CreateConstantBufferView(&cbvDesc, descriptor[D3D12_DESCRIPTOR_RANGE_TYPE_CBV]);
        }

        if (type & Type::Storage)
		{
             D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
                .Format                  = DXGI_FORMAT_UNKNOWN,
		        .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
		        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer                  = {
                    .FirstElement        = 0,
                    .NumElements         = UINT(GetSize() / byteStride),
		            .StructureByteStride = UINT(byteStride),
                    .Flags               = D3D12_BUFFER_SRV_FLAG_NONE,
                }
            };

            device->CreateShaderResourceView(*this, &srvDesc, descriptor[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]);

			D3D12_UNORDERED_ACCESS_VIEW_DESC desc {
                .Format        = DXGI_FORMAT_UNKNOWN,
			    .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
                .Buffer = {
				    .FirstElement         = 0,
				    .NumElements          = UINT(GetSize() / byteStride),
					.StructureByteStride  = UINT(byteStride),
				    .CounterOffsetInBytes = 0,
				    .Flags                = D3D12_BUFFER_UAV_FLAG_NONE,
                }
            };
			device->CreateUnorderedAccessView(*this, nullptr, &desc, descriptor[D3D12_DESCRIPTOR_RANGE_TYPE_UAV]);
		}
    }

#ifdef _DEBUG
	std::wstring name = L"Buffer_" + std::to_wstring(GetSize());
	resource->SetName(name.c_str());
#endif
}

Anonymous Buffer::GetBackendHandle() const
{
	return (void *) resource.Get();
}

GpuVirtualAddress Buffer::GetGpuVirtualAddress()
{
	return GetGPUVirtualAddress();
}

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
	*ppData = nullptr;
    D3D12_RANGE range = {
        .Begin = offset,
        .End   = offset + size
    };

    DX_CHECK(resource->Map(0, &range, ppData));
}

void Buffer::Unmap()
{
	resource->Unmap(0, nullptr);
}

void Buffer::SetName(const char *name)
{
	resource->SetName(std::filesystem::path(name).wstring().c_str());
}

}
}
