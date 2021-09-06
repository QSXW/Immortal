#pragma once

#include "Framework/Device.h"
#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class Device : public Immortal::Device
{
public:
    using Super = Immortal::Device;

public:
    Device();

    virtual void *Handle() override
    {
        return handle.Get();
    }

private:
    ComPtr<IDXGIFactory4> factory{ nullptr };
    ComPtr<ID3D12Device>  handle{ nullptr };

    static inline bool UseWarpDevice{ true };
};

}
}

