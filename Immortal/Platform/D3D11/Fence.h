#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Fence
{
public:
    using Primitive = ID3D11Fence;
    D3D11_OPERATOR_HANDLE()

public:
	Fence(Device *device, uint64_t initValue = 0u, D3D11_FENCE_FLAG flags = D3D11_FENCE_FLAG_NONE);

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

protected:
    HANDLE event = nullptr;
};

}
}
