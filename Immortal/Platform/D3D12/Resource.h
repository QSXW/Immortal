#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class Resource
{
public:
    Resource() :
        resource{ nullptr },
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
        IfNotNullThenRelease(resource);
        resource = nullptr;
        virtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    void SetName(const std::wstring &name)
    {
        resource->SetName(name.c_str());
    }

    bool Ready()
    {
        return !!resource;
    }

    operator ID3D12Resource*() const
    {
        return resource;
    }

protected:
    ID3D12Resource *resource;

    D3D12_GPU_VIRTUAL_ADDRESS virtualAddress;

    struct
    {
        D3D12_RESOURCE_STATES usage;
        D3D12_RESOURCE_STATES transition;
    } state;
};

}
}
