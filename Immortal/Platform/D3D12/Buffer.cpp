#include "Buffer.h"
#include "Device.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

Buffer::Buffer(const Buffer &other, const BindInfo &bindInfo) :
    Super{ bindInfo.type, bindInfo.size },
    context{},
    descriptor{ other.descriptor }
{
    virtualAddress = other.virtualAddress + bindInfo.offset;
}

Buffer::Buffer(RenderContext *context, const size_t size, const void *data, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size },
    context{ context }
{
    __Create();
    if (data)
    {
        Update(size, data);
    }
}

Buffer::Buffer(RenderContext *context, const size_t size, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size },
    context{ context }
{
    __Create();
}

Buffer::~Buffer()
{
    if (context)
    {
        context->RefResource(resource.Get());
    }
}

void Buffer::__Create()
{
    Device *device = context->GetAddress<Device>();

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
        .Width            = size,
        .Height           = 1,
        .DepthOrArraySize = 1,
        .MipLevels        = 1,
        .Format           = DXGI_FORMAT_UNKNOWN,
        .SampleDesc       = { .Count = 1, .Quality = 0 },
        .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags            = D3D12_RESOURCE_FLAG_NONE,
    };

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

    if (type & Type::Uniform)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = Desc();
        descriptor = context->AllocateDescriptor(DescriptorHeap::Type::ShaderResourceView);
        device->CreateView(&desc, descriptor);
    }
#ifdef _DEBUG
    resource->SetName(L"Buffer");
#endif
}

void Buffer::Update(uint64_t updateSize, const void *data, uint64_t offset)
{
    THROWIF(updateSize > size, SError::OutOfBound);

    uint8_t *memory = nullptr;
    Check(Map(rcast<void **>(&memory)));
    memcpy(memory + offset, data, updateSize);
    Unmap();
}

Buffer *Buffer::Bind(const BindInfo &bindInfo) const
{
    return new Buffer{ *this, bindInfo };
}

}
}
