#include "Queue.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Queue::Queue(Device *device, const Description &desc) :
    desc{ desc },
    nextFenceValue{ ncast<UINT64>(desc.Type) << 56 | 1 },
    lastCompletedFenceValue{ ncast<UINT64>(desc.Type) << 56 | 1 },
    commandAllocatorPool{ *device, desc.Type }
{
    Check(device->Create(&desc, &handle));
    handle->SetName(L"Command Queue");

    device->CreateFence(&fence, 0, D3D12_FENCE_FLAG_NONE);
    fence->SetName(L"Command Queue::Fence");
    fence->Signal((uint64_t)desc.Type << 56);

    fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    SLASSERT(fenceEventHandle != nullptr);

    uploadContext.Allocator = RequestCommandAllocator();
    uploadContext.Cmdlist.reset(new CommandList{
        device,
        CommandList::Type::Direct,
        uploadContext.Allocator
    });
    uploadContext.Cmdlist->Close();
    Discard(fence->GetCompletedValue(), uploadContext.Allocator);
}

Queue::~Queue()
{
    IfNotNullThenRelease(handle);
    IfNotNullThenRelease(fence);
}

}
}
