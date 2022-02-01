#include "Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Buffer::Buffer(Device *device, const size_t size, const void *data, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size }
{
    InternelCreate(device);
    Update(size, data);
}

Buffer::Buffer(Device * device, const size_t size, Type type) :
    Super{ type, type == Type::Uniform ? SLALIGN(size, 256) : size }
{
    InternelCreate(device);
}

Buffer::Buffer(Device *device, const size_t size, uint32_t binding) :
    Super{ Buffer::Type::Uniform, SLALIGN(size, 256) },
    binding{ binding }
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

    D3D12_RESOURCE_DESC desc{};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width              = size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // Zero is effectively 64KB also
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        &resource
    );

    virtualAddress = resource->GetGPUVirtualAddress();
}

void Buffer::Update(uint32_t updateSize, const void *data, uint32_t offset)
{
    THROWIF(updateSize > size, SError::OutOfBound);

    uint8_t *memory = nullptr;
    Map(rcast<void **>(&memory));
    {
        memcpy(memory + offset, data, updateSize);
    }
    Unmap();
}

}
}
