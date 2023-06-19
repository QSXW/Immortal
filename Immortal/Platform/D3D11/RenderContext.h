#pragma once

#include "Render/RenderContext.h"

#ifndef _MSC_VER
namespace Immortal
{
namespace D3D11
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
#include "RenderTarget.h"
#include "WindowCapture.h"
#include "ImGui/GuiLayer.h"

namespace Immortal
{
namespace D3D11
{

class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

    static constexpr int MAX_FRAME_COUNT{ 3 };

public:
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

	virtual WindowCapture *CreateWindowCapture() override;

	virtual void PushConstant(SuperGraphicsPipeline *super, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset) override;

	virtual void PushConstant(SuperComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset) override;

	virtual DescriptorBuffer *CreateImageDescriptor(uint32_t count) override;

	virtual DescriptorBuffer *CreateBufferDescriptor(uint32_t count) override;

	virtual void Draw(SuperGraphicsPipeline *pipeline) override;

	virtual void Dispatch(SuperComputePipeline *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void Blit(SuperTexture *texture) override;

	virtual void Begin(SuperRenderTarget *renderTarget) override;

	virtual void End() override;

public:
	template <class T>
	requires std::is_same_v<T, Device>    ||
	         std::is_same_v<T, Swapchain> ||
			 std::is_same_v<T, Window>    ||
			 std::is_same_v<T, RenderTarget>
	T *GetAddress()
	{
		if constexpr (std::is_same_v<T, Device>)
		{
			return device;
		}
		if constexpr (std::is_same_v<T, Swapchain>)
		{
			return swapchain;
		}
		if constexpr (std::is_same_v<T, Window>)
		{
			return desc.window;
		}
		if constexpr (std::is_same_v<T, RenderTarget>)
		{
			return renderTarget;
		}
	}

	template <class T>
	void Submit(T &&process)
	{
		auto context = device->GetContext();
		process(context);
	}

protected:
	void __Prepare();

	void __UpdateSwapchain();

protected:
    Super::Description desc{};

    URef<Device> device;

	URef<Swapchain> swapchain;

	URef<RenderTarget> renderTarget;
};

}
}

#endif
