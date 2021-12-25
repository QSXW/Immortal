#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class Resource
{
public:
    struct Description : public D3D12_RESOURCE_DESC
    {

    };

public:
    virtual ~Resource()
    {
        Destory();
    }

    virtual void Destory()
    {
        IfNotNullThenRelease(resource);
        gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    void Set(const std::wstring &name)
    {
        resource->SetName(name.c_str());
    }

    bool Ready()
    {
        return !!resource;
    }

    operator ID3D12Resource*()
    {
        return resource;
    }

protected:
    ID3D12Resource *resource{ nullptr };

    struct
    {
        D3D12_RESOURCE_STATES usage{ D3D12_RESOURCE_STATE_COMMON };
        D3D12_RESOURCE_STATES transition{ (D3D12_RESOURCE_STATES)-1 };
    } state;

    D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress{ D3D12_GPU_VIRTUAL_ADDRESS_NULL };
};

}
}
