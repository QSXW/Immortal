#pragma once

#include "Framework/Device.h"
#include "D3D12Common.h"

#include "Queue.h"

namespace Immortal
{
namespace D3D12
{

class Device : SuperDevice
{
public:
    using Super = SuperDevice;

public:
    Device(ComPtr<IDXGIFactory4> factory);

    ID3D12Device *Handle()
    {
        return handle.Get();
    }

    std::unique_ptr<Queue> CreateQueue(Queue::Description &desc)
    {
        return std::make_unique<Queue>(handle, desc);
    }

private:
    ComPtr<ID3D12Device>  handle{ nullptr };

    static inline bool UseWarpDevice{ true };
};

}
}
