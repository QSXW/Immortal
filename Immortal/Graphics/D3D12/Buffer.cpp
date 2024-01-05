#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Buffer::Buffer() :
    Super{},
    NonDispatchableHandle{}
{

}

Buffer::Buffer(Device *device, Type type, size_t size, const void *data) :
    Super{ type, type == Type::ConstantBuffer ? SLALIGN(size, 256) : size },
    NonDispatchableHandle{ device }
{
    Construct();
    if (data)
    {
		void *mapped = nullptr;
		Map(&mapped);
		memcpy(mapped, data, size);
		Unmap();
    }
}

Buffer::~Buffer()
{
    
}

void Buffer::Construct()
{
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ;

    D3D12_HEAP_PROPERTIES heapProperties = {
        .Type                 = D3D12_HEAP_TYPE_UPLOAD,
        .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        .CreationNodeMask     = 1,
        .VisibleNodeMask      = 1,
    };

    D3D12_RESOURCE_DESC desc = {
        .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment        = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, // Zero is effectively 64KB also
        .Width            = GetSize(),
        .Height           = 1,
        .DepthOrArraySize = 1,
        .MipLevels        = 1,
        .Format           = DXGI_FORMAT_UNKNOWN,
        .SampleDesc       = { .Count = 1, .Quality = 0 },
        .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags            = D3D12_RESOURCE_FLAG_NONE,
    };
    
    const auto &type = GetType();
    if (type & Type::Scratch)
    {
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    if (type & Type::AccelerationStructure)
    {
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    Check(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        state,
        nullptr,
        &resource
    ));

    virtualAddress = resource->GetGPUVirtualAddress();

    if (type & Type::ConstantBuffer)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = Desc();
		descriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        device->CreateConstantBufferView(&desc, descriptor);
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

void Buffer::Map(void **ppData, size_t size, uint64_t offset)
{
    D3D12_RANGE range = {
        .Begin = offset,
        .End   = offset + size
    };

    if (FAILED(resource->Map(0, &range, ppData)))
    {
		*ppData = nullptr;
    }
}

void Buffer::Unmap()
{
	resource->Unmap(0, nullptr);
}

}
}
