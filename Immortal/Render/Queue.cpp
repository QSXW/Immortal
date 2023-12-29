#pragma once

#include "Queue.h"

namespace Immortal
{

void Queue::Submit(CommandBuffer *pCommandBuffer, GPUEvent *pSignalEvent, Swapchain *swapchain)
{
    CommandBuffer *ppCommandBuffer[] = {
        pCommandBuffer
    };

    GPUEvent *ppSignalEvent[] = {
        pSignalEvent
    };

    Submit(ppCommandBuffer, 1, ppSignalEvent, pSignalEvent ? 1 : 0, swapchain);
}

}
