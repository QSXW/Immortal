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

private:
    void InternelCreate(const Device *device);
};

}
}
