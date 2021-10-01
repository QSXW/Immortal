#pragma once

#include "Render/RenderContext.h"
#include "D3D12Common.h"

#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"

namespace Immortal
{
namespace D3D12
{

class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

    static constexpr int MAX_FRAME_COUNT{ 3 };

public:
    RenderContext(const void *handle);

    RenderContext(Description &desc);

    ~RenderContext();

    void INIT();

    template <class T>
    T Get()
    {
        if constexpr (typeof<T, Swapchain*>())
        {
            return swapchain.get();
        }
    }

private:
    Super::Description desc{};

    HWND hWnd{ nullptr };

    HMODULE hModule{ nullptr };

    ComPtr<IDXGIFactory4> factory{ nullptr };

    std::unique_ptr<Device> device;

    std::unique_ptr<Queue> queue;

    std::unique_ptr<Swapchain> swapchain;

    std::unique_ptr<DescriptorPool> renderTargetViewDescriptorHeap;

    std::unique_ptr<DescriptorPool> shaderResourceViewDescriptorHeap;
    
    UINT frameIndex{ 0 };

    UINT renderTargetViewDescriptorSize{ 0 };

    ComPtr<ID3D12Resource> renderTargets[MAX_FRAME_COUNT]{ nullptr };

    ComPtr<ID3D12CommandAllocator>commandAllocator;
};

}
}
