#pragma once

#include "Render/RenderContext.h"

#ifndef _MSC_VER
namespace Immortal
{
namespace D3D12
{

class RenderContext : public SuperRenderContext
{
public:
    RenderContext(const Description &desc)
    {
        ThrowIf(true, "D3D12 is not supported on this platform");
    }
};

}
}
#else

#include <array>
#include <concepts>

#include "Common.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorHeap.h"
#include "CommandPool.h"
#include "CommandAllocator.h"
#include "Queue.h"
#include "Fence.h"
#include "Math/Vector.h"

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

    RenderContext(const Description &desc);

    ~RenderContext();

    virtual GuiLayer *CreateGuiLayer() override;

    void Setup();

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<DXGI_FORMAT, T>())
        {
            return desc.format;
        }
    }

    template <class T>                      
    requires std::is_same_v<T, Device> || std::is_same_v<T, Swapchain>   ||
             std::is_same_v<T, Queue>  || std::is_same_v<T, CommandList> ||
             std::is_same_v<T, Window> || std::is_same_v<T, ID3D12CommandAllocator*>
    inline constexpr T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device.get();
        }
        if constexpr (IsPrimitiveOf<Swapchain, T>())
        {
            return swapchain.get();
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return queue.get();
        }
        if constexpr (IsPrimitiveOf<CommandList, T>())
        {
            return commandList.get();
        }
        if constexpr (IsPrimitiveOf<Window, T>())
        {
            return desc.WindowHandle;
        }
        if constexpr (IsPrimitiveOf<ID3D12CommandAllocator*, T>())
        {
            return commandAllocator;
        }
    }

    UINT FrameSize()
    {
        return desc.FrameCount;
    }

    DescriptorPool *ShaderResourceViewDescritorHeap()
    {
        return shaderVisibleDescriptorAllocator.GetAddress<DescriptorPool>();
    }

    Vector2 Extent()
    {
        return Vector2{
            ncast<float>(desc.Width),
            ncast<float>(desc.Height)
        };
    }

    void CheckDisplayHDRSupport();

    void EnsureSwapChainColorSpace(Swapchain::BitDepth bitDepth, bool enableST2084);

    void SetHDRMetaData(float MaxOutputNits = 1000.0f, float MinOutputNits = 0.001f, float MaxCLL = 2000.0f, float MaxFALL = 500.0f);

    void WaitForGPU();

    void WaitForNextFrameResources();

    void UpdateSwapchain(UINT width, UINT height);

    void CopyDescriptorHeapToShaderVisible();

    UINT WaitForPreviousFrame();

private:
    Super::Description desc{};

    ComPtr<IDXGIFactory4> dxgiFactory{ nullptr };

    std::unique_ptr<Device> device;

    std::unique_ptr<Queue> queue;

    std::unique_ptr<Swapchain> swapchain;

    HANDLE swapchainWritableObject{ nullptr };

    std::unique_ptr<CommandList> commandList;

    ID3D12CommandAllocator *commandAllocator[MAX_FRAME_COUNT];

    std::unique_ptr<Fence> fence;

    uint64_t fenceValues[MAX_FRAME_COUNT]{ 0 };

    uint32_t frameIndex = 0;

    struct
    {
        UINT hdrMetaDataPoolIdx{ 0 };

        bool hdrSupport{ false };

        bool enableST2084{ false };

        float referenceWhiteNits{ 80.0f };

        UINT rootConstants[RootConstantsCount];

        Swapchain::BitDepth bitDepth{ Swapchain::BitDepth::_8 };

        DXGI_COLOR_SPACE_TYPE currentColorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };
    } color;

    Rect windowBounds;

    ErrorHandle error;

public:
    static Device *UnlimitedDevice;

    static DescriptorAllocator descriptorAllocators[U32(DescriptorPool::Type::Quantity)];

    static DescriptorAllocator shaderVisibleDescriptorAllocator;

    static DescriptorPool *Request(DescriptorPool::Type type)
    {
        return DescriptorAllocator::Request(UnlimitedDevice, type);
    }

    static inline CPUDescriptor AllocateDescriptor(DescriptorPool::Type type, uint32_t count = 1)
    {
        return descriptorAllocators[U32(type)].Allocate(UnlimitedDevice, count);
    }

    static inline Descriptor AllocateShaderVisibleDescriptor(uint32_t count = 1)
    {
        return shaderVisibleDescriptorAllocator.Allocate(UnlimitedDevice);
    }
};

}
}

#endif
