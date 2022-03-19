#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Fence
{
public:
    enum class Type
    {
        None               = D3D12_FENCE_FLAG_NONE,
        Shared             = D3D12_FENCE_FLAG_SHARED,
        SharedCrossAdapter = D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
        NonMonitored       = D3D12_FENCE_FLAG_NON_MONITORED
    };

public:
    Fence(Device *device, uint64_t initValue = 0u, Type type = Type::None);

    ~Fence();

    void Destroy();

public:
    HRESULT SetCompletion(uint64_t value) const
    {
        return handle->SetEventOnCompletion(value, event);
    }

    uint64_t GetCompletion() const
    {
        return handle->GetCompletedValue();
    }

    void Wait(DWORD milliseconds = INFINITE, BOOL alertable = FALSE)
    {
        WaitForSingleObjectEx(event, milliseconds, alertable);
    }

    HANDLE GetEvent() const
    {
        return event;
    }

    operator ID3D12Fence*() const
    {
        return handle.Get();
    } 

private:
    ComPtr<ID3D12Fence> handle;

    HANDLE event = nullptr;
};

}
}
