#include "GPUEvent.h"

namespace Immortal
{
namespace OpenGL
{

GPUEvent::GPUEvent() :
    value{}
{

}

GPUEvent::~GPUEvent()
{

}

void GPUEvent::Signal(uint64_t value)
{

}

void GPUEvent::Wait(uint64_t value, uint64_t timeout)
{

}

void GPUEvent::Wait(uint64_t timeout)
{

}

uint64_t GPUEvent::GetCompletionValue()
{
	return 0;
}

uint64_t GPUEvent::GetSyncPoint()
{
	return value;
}

}
}
