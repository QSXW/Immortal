#include "RenderContext.h"

#include "Framework/Utils.h"
#include "Descriptor.h"
#include "GuiLayer.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace D3D11
{

RenderContext::RenderContext(const Description &descrition) :
    desc{ descrition }
{
    __Prepare();
}

RenderContext::~RenderContext()
{

}

void RenderContext::__Prepare()
{
	device = new Device(desc.deviceId);

    DXGI_SWAP_CHAIN_DESC dxgiDesc = {
		.BufferDesc = {
		    .Width            = desc.width,
		    .Height           = desc.height,
		    .RefreshRate      = {
		        .Numerator   = 0,
		        .Denominator = 1,
		    },
		    .Format           = desc.format,
		    .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		    .Scaling          = DXGI_MODE_SCALING_UNSPECIFIED,
		},
		.SampleDesc = {
		    .Count   = 1,
		    .Quality = 0,
		},
		.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount  = U32(desc.presentMode),
		.OutputWindow = (HWND)desc.window->Primitive(),
        .Windowed     = true,
	    .SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	    .Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
    };

	Check(device->CreateSwapchain(swapchain = new Swapchain, &dxgiDesc));
	__UpdateSwapchain();
}

void RenderContext::__UpdateSwapchain()
{
	Ref<Image> image = new Image{desc.format};
	Check(swapchain->GetBuffer(0, &(*image)));

	renderTarget = new RenderTarget{device, image};
}

void RenderContext::PrepareFrame()
{

}

void RenderContext::SwapBuffers()
{
	Check(swapchain->Present(1, 0));
}

void RenderContext::OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	renderTarget.Reset();
	Check(swapchain->ResizeBuffers(width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH, U32(desc.presentMode)));
	__UpdateSwapchain();
}

SuperGuiLayer *RenderContext::CreateGuiLayer()
{
    return new GuiLayer{ this };
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, const void *data, Buffer::Type type)
{
	return new Buffer{ device, size, data, type };
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, Buffer::Type type)
{
	return new Buffer{ device, size, type };
}

SuperBuffer *RenderContext::CreateBuffer(const size_t size, uint32_t binding)
{
	return new Buffer{ device, size, binding };
}

SuperShader *RenderContext::CreateShader(const std::string &filepath, Shader::Type type)
{
	return new Shader{ device, filepath, type };
}

SuperGraphicsPipeline *RenderContext::CreateGraphicsPipeline(Ref<SuperShader> shader)
{
	return new GraphicsPipeline{ device, shader };
}

SuperComputePipeline *RenderContext::CreateComputePipeline(SuperShader *shader)
{
	return new ComputePipeline{ device, shader };
}

SuperTexture *RenderContext::CreateTexture(const std::string &filepath, const Texture::Description &description)
{
	return new Texture{ device, filepath, description };
}

SuperTexture *RenderContext::CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description)
{
	return new Texture{ device, width, height, data, description };
}

SuperRenderTarget *RenderContext::CreateRenderTarget(const RenderTarget::Description &description)
{
	return new RenderTarget{ device, description};
}

void RenderContext::PushConstant(SuperGraphicsPipeline *super, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset)
{

}

void RenderContext::PushConstant(SuperComputePipeline *super, uint32_t size, const void *data, uint32_t offset)
{

}

DescriptorBuffer *RenderContext::CreateImageDescriptor(uint32_t count)
{
	DescriptorBuffer *ret = new DescriptorBuffer;
	ret->Request<ID3D11ShaderResourceView *>(count);

	return ret;
}

DescriptorBuffer *RenderContext::CreateBufferDescriptor(uint32_t count)
{
	return CreateImageDescriptor(count);
}

void RenderContext::Draw(SuperGraphicsPipeline *_pipeline)
{
	GraphicsPipeline *pipeline = dynamic_cast<GraphicsPipeline *>(_pipeline);
	Submit([=, this](CommandList *cmdlist) {
		pipeline->Draw(cmdlist);
	});
}

void RenderContext::Begin(SuperRenderTarget *_renderTarget)
{
	RenderTarget *renderTarget = dynamic_cast<RenderTarget *>(_renderTarget);
	Submit([=, this](CommandList *cmdlist) {
		auto &colorBuffers = renderTarget->GetColorBuffer();
		auto &depthBuffer  = renderTarget->GetDepthBuffer();

		cmdlist->OMSetRenderTargets(colorBuffers.size(), renderTarget->GetViews(), depthBuffer.descriptor);
		for (auto &colorBuffer : colorBuffers)
		{
			cmdlist->ClearRenderTargetView(colorBuffer.descriptor, (FLOAT *) renderTarget->ClearColor());
		}
		cmdlist->ClearDepthStencilView(depthBuffer.descriptor, D3D11_CLEAR_DEPTH, 1.0f, 0);

		D3D11_VIEWPORT viewport = {
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width    = FLOAT(renderTarget->Width()),
			.Height   = FLOAT(renderTarget->Height()),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		};
		cmdlist->RSSetViewports(1, &viewport);
	});
}

void RenderContext::End()
{

}

}
}
