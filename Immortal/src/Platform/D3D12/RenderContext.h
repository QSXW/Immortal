#pragma once

#include "Render/RenderContext.h"
#include "D3D12Common.h"

#include <array>

#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"
#include "CommandPool.h"
#include "CommandAllocator.h"

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
        if constexpr (typeof<T, HWND>())
        {
            return rcast<HWND>(desc.WindowHandle);
        }
        if constexpr (typeof<T, ID3D12Device*>())
        {
            return device->Handle();
        }
        if constexpr (typeof<T, DXGI_FORMAT>())
        {
            return NORMALIZE(desc.format);
        }
        if constexpr (typeof<T, Queue *>())
        {
            return queue.get();
        }
        if constexpr (typeof<T, CommandList *>())
        {
            return commandList.get();
        }
        if constexpr (typeof<T, CommandAllocator *>())
        {
            return commandAllocator[0].get();
        }
    }

    UINT FrameSize()
    {
        return desc.FrameCount;
    }

    DescriptorPool *ShaderResourceViewDescritorHeap()
    {
        return shaderResourceViewDescriptorHeap.get();
    }

    ID3D12Resource *RenderTarget(UINT backBufferIndex)
    {
        return renderTargets[backBufferIndex];
    }

    Descriptor &&RenderTargetDescriptor(INT backBufferIndex)
    {
        return Descriptor{
                 renderTargetViewDescriptorHeap->Get<D3D12_CPU_DESCRIPTOR_HANDLE>(),
                 backBufferIndex,
                 renderTargetViewDescriptorSize
            };
    }

    Vector2 Extent()
    {
        return Vector2{
            ncast<float>(desc.Width),
            ncast<float>(desc.Height)
        };
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
    
    std::unique_ptr<CommandList> commandList;

    UINT frameIndex{ 0 };

    UINT renderTargetViewDescriptorSize{ 0 };

    ID3D12Resource *renderTargets[MAX_FRAME_COUNT]{ nullptr };

    std::array<std::unique_ptr<CommandAllocator>, MAX_FRAME_COUNT> commandAllocator;

    ComPtr<ID3D12Fence> fence;

    HANDLE fenceEvent{ nullptr };

public:
    ErrorHandle error;
};

}
}
