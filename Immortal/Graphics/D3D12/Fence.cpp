#include "Fence.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Fence::Fence(Device *device, uint64_t initValue, Type type)
{
    DX_CHECK(device->CreateFence(&handle, initValue, D3D12_FENCE_FLAGS(type)));

    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event)
    {
        DX_CHECK(HRESULT_FROM_WIN32(GetLastError()));
    }
}

Fence::~Fence()
{
    Destroy();
}

void Fence::Destroy()
{
    if (event)
    {
        CloseHandle(event);
        event = nullptr;
    }
    handle.Reset();
}

}
}
