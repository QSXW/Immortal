#include "impch.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{
static inline DXGI_FORMAT NORMALIZE(Format format)
{
    static DXGI_FORMAT map[] = {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R16G16B16A16_FLOAT
    };

    return map[ncast<int>(format)];
}

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

        swapchain = std::make_unique<Swapchain>(factory, queue->Handle(), hWnd, swapchainDesc);
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
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor {
            renderTargetViewDescriptorHeap->CPUDescriptorHandleForHeapStart()
            };

        for (UINT i = 0; i < desc.FrameCount; i++)
        {
            swapchain->AccessBackBuffer(i, renderTargets[i]);
            device->CreateRenderTargetView(renderTargets[i], nullptr, renderTargetViewDescriptor);
        }
    }

}

}
}
