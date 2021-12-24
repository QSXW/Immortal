#include "Buffer.h"

namespace Immortal
{
namespace D3D12
{

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type) :
    Super{ type, size }
{
    InternelCreate(device);
    Update(size, data);
}

Buffer::Buffer(Device * device, const size_t size, Type type) :
    Super{ type, size }
{
    InternelCreate(device);
}

Buffer::~Buffer()
{

}

void Buffer::InternelCreate(const Device *device)
{
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask     = 1;
    heapProperties.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resourceDesciption{};
    resourceDesciption.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesciption.Width              = size;
    resourceDesciption.Height             = 1;
    resourceDesciption.DepthOrArraySize   = 1;
    resourceDesciption.MipLevels          = 1;
    resourceDesciption.Format             = DXGI_FORMAT_UNKNOWN;
    resourceDesciption.SampleDesc.Count   = 1;
    resourceDesciption.SampleDesc.Quality = 0;
    resourceDesciption.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesciption.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // Zero is effectively 64KB also
    resourceDesciption.Flags              = D3D12_RESOURCE_FLAG_NONE;

    device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesciption,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        &resource
    );

    gpuVirtualAddress = resource->GetGPUVirtualAddress();
}

void Buffer::Update(uint32_t updateSize, const void *data)
{
    ThrowIf(updateSize > size, SError::OutOfBound);
    uint8_t *memory = nullptr;
    Map(rcast<void **>(&memory));
    memcpy(memory, data, updateSize);
    Unmap();
}

}
}
