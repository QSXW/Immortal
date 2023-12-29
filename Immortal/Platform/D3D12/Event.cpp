#include "Event.h"

namespace Immortal
{
namespace D3D12
{

GPUEvent::GPUEvent(Device *device) :
    value{ 0 },
    fence{ device, value }
{
#ifdef _DEBUG
	((ID3D12Fence *)fence)->SetName(L"GPUEvent::Fence");
#endif
}

GPUEvent::~GPUEvent()
{
}

void GPUEvent::Signal(uint64_t value)
{
	(void)value;
}

void GPUEvent::Wait(uint64_t value, uint64_t timeout)
{
	uint64_t completion = GetCompletionValue();
	if (completion < value)
	{
		fence.SetCompletion(value);
		fence.Wait(timeout);
	}
}

void GPUEvent::Wait(uint64_t timeout)
{
	Wait(value, timeout);
}

uint64_t GPUEvent::GetCompletionValue()
{
	return fence.GetCompletion();
}

uint64_t GPUEvent::GetSyncPoint()
{
	return value;
}

}
}
