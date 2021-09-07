#include "impch.h"
#include "RenderFrame.h"

namespace Immortal
{
namespace Vulkan
{
    RenderFrame::RenderFrame(Device *device, std::unique_ptr<RenderTarget> &&renderTarget, size_t threadCount) :
        device{ device },
        fencePool{ device },
        semaphorePool{ device },
        renderTarget{ std::move(renderTarget) },
        threadCount{ threadCount }
    {
        for (UINT32 i = 0; i < threadCount; i++)
        {

        }
    }
}
}
