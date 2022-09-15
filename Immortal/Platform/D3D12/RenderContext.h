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
#include "CommandList.h"
#include "CommandAllocator.h"
#include "Queue.h"
#include "Fence.h"
#include "Math/Vector.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "Texture.h"
#include "Pipeline.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{
namespace D3D12
{

struct RenderEntry
{
	URef<Queue> queue;
	URef<CommandList> cmdlist;
};

class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

    static constexpr int MAX_FRAME_COUNT{ 3 };

public:
    RenderContext(const void *handle);

    RenderContext(const Description &desc);

    ~RenderContext();

    virtual void SwapBuffers() override;

	virtual void PrepareFrame() override;

	virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    virtual SuperGuiLayer *CreateGuiLayer() override;

	virtual SuperBuffer *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, Buffer::Type type) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, uint32_t binding) override;

	virtual SuperShader *CreateShader(const std::string &filepath, Shader::Type type) override;

	virtual SuperGraphicsPipeline *CreateGraphicsPipeline(Ref<SuperShader> shader) override;

	virtual SuperComputePipeline *CreateComputePipeline(SuperShader *shader) override;

	virtual SuperTexture *CreateTexture(const std::string &filepath, const Texture::Description &description = {}) override;

	virtual SuperTexture *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description = {}) override;

	virtual SuperRenderTarget *CreateRenderTarget(const RenderTarget::Description &description) override;

	virtual void PushConstant(SuperGraphicsPipeline *super, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset) override;

	virtual void PushConstant(SuperComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset) override;

	virtual DescriptorBuffer *CreateImageDescriptor(uint32_t count) override;

	virtual DescriptorBuffer *CreateBufferDescriptor(uint32_t count) override;

	virtual void Draw(SuperGraphicsPipeline *pipeline) override;

	virtual void Begin(SuperRenderTarget *renderTarget) override;

	virtual void End() override;

	void PushConstant(Pipeline *super, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset);

public:
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
            return device;
        }
        if constexpr (IsPrimitiveOf<Swapchain, T>())
        {
            return swapchain;
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return graphicsDispatcher->GetAddress<Queue>();
        }
        if constexpr (IsPrimitiveOf<CommandList, T>())
        {
            return graphicsDispatcher->GetCommandList();
        }
        if constexpr (IsPrimitiveOf<Window, T>())
        {
            return desc.WindowHandle;
        }
    }

    UINT FrameSize()
    {
        return desc.FrameCount;
    }

    DescriptorPool *ShaderResourceViewDescritorHeap()
    {
        return shaderVisibleDescriptorAllocator->GetAddress<DescriptorPool>();
    }

    CPUDescriptor AllocateDescriptor(DescriptorPool::Type type, uint32_t count = 1)
    {
        return descriptorAllocators[U32(type)]->Allocate(device, count);
    }

    Descriptor AllocateShaderVisibleDescriptor(uint32_t count = 1)
    {
        return shaderVisibleDescriptorAllocator->Allocate(device);
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

    void UpdateSwapchain(UINT width, UINT height);

    void CopyDescriptorHeapToShaderVisible();

    UINT WaitForPreviousFrame();

    template <class T>
    void Submit(T &&process)
    {
        process(commandList);
    }

    template <class T>
    void Transfer(T &&process)
    {
        auto cmdlist = transferDispatcher->GetCommandList();
        process(graphicsDispatcher->GetCommandList(), cmdlist);
    }

    template <class T>
    void Compute(T &&process)
    {
        process(graphicsDispatcher->GetCommandList());
    }

protected:
	void __InitQueue();

private:
    Super::Description desc{};

    URef<Device> device;

    CommandList *commandList{};
    URef<CommandListDispatcher> graphicsDispatcher;
    URef<CommandListDispatcher> transferDispatcher;

    URef<Swapchain> swapchain;

    HANDLE swapchainWritableObject{ nullptr };

    uint64_t fenceValues[MAX_FRAME_COUNT]{ 0 };

    uint32_t frameIndex = 0;

    URef<DescriptorAllocator> descriptorAllocators[U32(DescriptorPool::Type::Quantity)];

    URef<DescriptorAllocator> shaderVisibleDescriptorAllocator;

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

	std::array<Barrier<BarrierType::Transition>, 8> barriers;

	uint32_t activeBarrier = 0;
};

}
}

#endif
