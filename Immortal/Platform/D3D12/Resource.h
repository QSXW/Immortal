#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class Resource
{
public:
    using Primitive = ID3D12Resource;
	D3D12_OPERATOR_PRITIMIVE(resource)

public:
    Resource() :
        resource{},
        virtualAddress{ D3D12_GPU_VIRTUAL_ADDRESS_NULL },
        state{ D3D12_RESOURCE_STATE_COMMON, (D3D12_RESOURCE_STATES)-1 }
    {

    }

    ~Resource()
    {
        Destory();
    }

    Resource(Resource &&other) :
        resource{ other.resource },
        virtualAddress{ other.virtualAddress },
        state{ other.state }
    {
        other.resource = nullptr;
        other.virtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    Resource &operator=(Resource &&other)
    {
        resource       = other.resource;
        virtualAddress = other.virtualAddress;
        state          = other.state;

        other.resource = nullptr;
        other.virtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    virtual void Destory()
    {
		resource.Reset();
        virtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    operator bool() const
    {
        return !!resource;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return resource->GetGPUVirtualAddress();
    }

protected:
    D3D12_GPU_VIRTUAL_ADDRESS virtualAddress;

    struct
    {
        D3D12_RESOURCE_STATES usage;
        D3D12_RESOURCE_STATES transition;
    } state;
};

}
}
