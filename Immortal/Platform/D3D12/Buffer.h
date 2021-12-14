#pragma once

#include "Common.h"
#include "Render/Buffer.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

class Buffer : public SuperBuffer
{
public:
    using Super = SuperBuffer;

    using HandleType = ID3D12Resource*;

public:
    Buffer(Device *device, const size_t size, const void *data, Type type);

    Buffer(Device *device, const size_t size, Type type);

    operator const HandleType() const
    {
        return handle;
    }

    virtual void Update(uint32_t size, const void *data) override;

    void Map(void **data)
    {
        D3D12_RANGE range{ 0, size };
        handle->Map(0, &range, data);
    }

    void Unmap()
    {
        D3D12_RANGE range{ 0, size };
        handle->Unmap(0, &range);
    }

private:
    void InternelCreate(const Device *device);

private:
    HandleType handle{ nullptr };

    D3D12_RESOURCE_STATES usageState{ D3D12_RESOURCE_STATE_COMMON };

    D3D12_RESOURCE_STATES transitioningState{ D3D12_RESOURCE_STATE_COMMON };

    D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress{ 0  };
};

}
}
