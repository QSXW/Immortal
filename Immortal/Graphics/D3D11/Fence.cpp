#include "Fence.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

Fence::Fence(Device *device, uint64_t initValue, D3D11_FENCE_FLAG flags)
{
	device->CreateFence(&handle, initValue, flags);

    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event)
    {
        Check(HRESULT_FROM_WIN32(GetLastError()));
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
