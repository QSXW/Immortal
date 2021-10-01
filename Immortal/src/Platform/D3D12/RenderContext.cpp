#include "impch.h"
#include "RenderContext.h"

#include "Descriptor.h"

namespace Immortal
{
namespace D3D12
{

RenderContext::RenderContext(Description &descrition) :
    desc{ descrition }
{
    INIT();
}

RenderContext::RenderContext(const void *handle)
{
    INIT();
}

RenderContext::~RenderContext()
{
    IfNotNullThen<FreeLibrary>(hModule);
}

void RenderContext::INIT()
{
    hWnd = rcast<HWND>(this->desc.WindowHandle);

    uint32_t dxgiFactoryFlags = 0;
#if SLDEBUG
    hModule = LoadLibrary(L"D3d12Core.dll");
    ComPtr<ID3D12Debug> debugController;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        LOG::INFO("Enable Debug Layer: {0}", rcast<void*>(debugController.Get()));
    }
#endif

    Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)), "Failed to create DXGI Factory");
    device = std::make_unique<Device>(factory);

    {
        Queue::Description queueDesc{};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

        queue = device->CreateQueue(queueDesc);
    }

    {
        Swapchain::Description swapchainDesc{};
        CleanObject(&swapchainDesc);
        swapchainDesc.BufferCount       = desc.FrameCount;
        swapchainDesc.Width             = desc.Width;
        swapchainDesc.Height            = desc.Height;
        swapchainDesc.Format            = NORMALIZE(desc.format);
        swapchainDesc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.SampleDesc.Count  = 1;
        swapchainDesc.Flags             = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

        swapchain = std::make_unique<Swapchain>(factory, queue->Handle(), hWnd, swapchainDesc);
        swapchain->SetMaximumFrameLatency(desc.FrameCount);
    }

    Check(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

    frameIndex = swapchain->AcquireCurrentBackBufferIndex();

    {
        DescriptorPool::Description renderTargetViewDesc{
            DescriptorPool::Type::RenderTargetView,
            desc.FrameCount,
            DescriptorPool::Flag::None
            };

        renderTargetViewDescriptorHeap = std::make_unique<DescriptorPool>(
            device->Handle(),
            renderTargetViewDesc
            );

        DescriptorPool::Description shaderResourceViewDesc{
            DescriptorPool::Type::ShaderResourceView,
            1,
            DescriptorPool::Flag::ShaderVisible
            };

        shaderResourceViewDescriptorHeap = std::make_unique<DescriptorPool>(
            device->Handle(),
            shaderResourceViewDesc
            );

        renderTargetViewDescriptorSize = device->DescriptorHandleIncrementSize(
            renderTargetViewDesc.Type
            );
    }

    {
        Descriptor renderTargetViewDescriptor {
            renderTargetViewDescriptorHeap->CPUDescriptorHandleForHeapStart()
            };

        for (UINT i = 0; i < desc.FrameCount; i++)
        {
            swapchain->AccessBackBuffer(i, &renderTargets[i]);
            device->CreateRenderTargetView(renderTargets[i], nullptr, renderTargetViewDescriptor);
            renderTargetViewDescriptor.Offset(1, renderTargetViewDescriptorSize);
        }
    }

    for (UINT i = 0; i < desc.FrameCount; i++)
    {
        commandAllocator[i] = std::make_unique<CommandAllocator>(device.get(), CommandList::Type::Direct);
    }
 
    commandList = std::make_unique<CommandList>(
        device.get(),
        CommandList::Type::Direct,
        commandAllocator[0].get()
        );

    commandList->Close();

    device->CreateFence(&fence);

    fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (fenceEvent)
    {
        error.Upload("Unable to intialize Fence Event");
    }
}

}
}
