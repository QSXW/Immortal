#include "CommandBuffer.h"
#include "Buffer.h"
#include "Device.h"
#include "DescriptorSet.h"
#include "Texture.h"
#include "Pipeline.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace D3D11
{

CommandBuffer::CommandBuffer(Device *device) :
    NonDispatchableHandle{ device },
    handle{ device->GetContext<ID3D11DeviceContext4>() },
    pushConstantBuffer{}
{
	pushConstantBuffer = (Buffer *)device->CreateBuffer(MAX_PUSH_CONSTANT_SIZE, BufferType::PushConstant);
}

CommandBuffer::~CommandBuffer()
{
	pushConstantBuffer.Reset();
	handle.Reset();
}

bool CommandBuffer::IsActive()
{
	return true;
}

void CommandBuffer::Reset()
{

}

void CommandBuffer::Begin()
{

}

void CommandBuffer::End()
{

}

void CommandBuffer::Close()
{

}

void CommandBuffer::BeginEvent(const char *pData, size_t size)
{

}

void CommandBuffer::EndEvent()
{

}

void CommandBuffer::SetPipeline(SuperPipeline *_pipeline)
{
	pipeline = InterpretAs<Pipeline>(_pipeline);
	Submit([=, this] {
		pipeline->SetPiplineState(handle.Get());
	});
}

void CommandBuffer::SetDescriptorSet(SuperDescriptorSet *_descriptorSet)
{
	DescriptorSet *descriptorSet = InterpretAs<DescriptorSet>(_descriptorSet);
	Submit([=, this] {
		auto &bufferDescriptorTable = descriptorSet->GetBufferDescriptorTable();
		for (auto &[slot, buffer] : bufferDescriptorTable)
		{
			handle->VSSetConstantBuffers(slot, 1, &buffer);
			handle->PSSetConstantBuffers(slot, 1, &buffer);
		}

		auto &shaderResourceViewTable = descriptorSet->GetShaderResourceViewTable();
		for (auto &[slot, shaderResourceView] : shaderResourceViewTable)
		{
			handle->PSSetShaderResources(slot, 1, &shaderResourceView);
		}

		auto &samplerDescriptorTable = descriptorSet->GetSamplerDescriptorTable();
		for (auto &[slot, samplerState] : samplerDescriptorTable)
		{
			handle->PSSetSamplers(slot, 1, &samplerState);
		}
	});
}

void CommandBuffer::SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **_ppBuffer, uint32_t /*strideInBytes*/)
{
	Buffer **ppBuffer = (Buffer **)_ppBuffer;
	Submit([=, this] {
		LightArray<ID3D11Buffer *> buffers;
		buffers.resize(bufferCount);

		LightArray<uint32_t> strides;
		strides.resize(bufferCount);

		LightArray<uint32_t> offsets;
		offsets.resize(bufferCount);

		GraphicsPipeline *graphicsPipeline = InterpretAs<GraphicsPipeline>(pipeline);
		for (uint32_t i = 0; i < bufferCount; i++)
		{
			buffers[i] = *ppBuffer[0];
			offsets[i] = ppBuffer[i]->GetOffset();
			strides[i] = graphicsPipeline->GetStride();
		}
		handle->IASetVertexBuffers(firstSlot, bufferCount, buffers.data(), strides.data(), offsets.data());
	});
}

void CommandBuffer::SetIndexBuffer(SuperBuffer *_buffer, Format format)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	Submit([=, this] {
		handle->IASetIndexBuffer(*buffer, format, buffer->GetOffset());
	});
}

void CommandBuffer::SetScissors(uint32_t count, const Rect2D *pScissor)
{
	D3D11_RECT rect = *(D3D11_RECT *)pScissor;
	Submit([=, this] {
		handle->RSSetScissorRects(1, &rect);
	});
}

void CommandBuffer::SetBlendFactor(const float factor[4])
{
	Submit([=, this] {
		GraphicsPipeline *graphicsPipeline = InterpretAs<GraphicsPipeline>(pipeline);
		graphicsPipeline->SetBlendFactor(handle.Get(), factor);
	});
}

void CommandBuffer::PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset)
{
	if (size > pushConstantBuffer->GetSize())
	{
		LOG::ERR("data size `{}` is greater to the max push constant size `{}`", size, pushConstantBuffer->GetSize());
		throw std::runtime_error("Out of Range!");
	}

	if (offset > 0)
	{
		LOG::ERR("D3D11 backend: push constant buffer is not support partial update. Please try to rescale offset ot 0!");
	}
	handle->UpdateSubresource(*pushConstantBuffer, 0, nullptr, pData, size, 0);

	Submit([=, this] {
		ID3D11Buffer *buffer = *pushConstantBuffer;
		if (stage == ShaderStage::Vertex)
		{
			auto &pushConstantDesc = pipeline->GetPushConstantDesc(PipelineStage::Vertex);
			handle->VSSetConstantBuffers(pushConstantDesc.Binding, 1, &buffer);
		}
		else if (stage == ShaderStage::Pixel)
		{
			auto &pushConstantDesc = pipeline->GetPushConstantDesc(PipelineStage::Pixel);
			handle->PSSetConstantBuffers(pushConstantDesc.Binding, 1, &buffer);
		}
		else if (stage == ShaderStage::Compute)
		{
			auto &pushConstantDesc = pipeline->GetPushConstantDesc(PipelineStage::Compute);
			handle->CSSetConstantBuffers(pushConstantDesc.Binding, 1, &buffer);
		}
	});
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const float *pClearColor)
{
	RenderTarget *renderTarget = InterpretAs<RenderTarget>(_renderTarget);
	Submit([=, this] {
		auto &colorAttachments = renderTarget->GetColorAttachments();
		auto &depthAttachment  = renderTarget->InternalGetDepthAttachment();

		ID3D11DepthStencilView *depthStencilView = depthAttachment.GetDSV();
		LightArray<ID3D11RenderTargetView *> renderTargetViews;
		renderTargetViews.resize(colorAttachments.size());

		for (size_t i = 0; i < colorAttachments.size(); i++)
		{
			renderTargetViews[i] = colorAttachments[i].GetRTV();
		}

		handle->OMSetRenderTargets(uint32_t(renderTargetViews.size()), renderTargetViews.data(), depthStencilView);
		for (auto &renderTargetView : renderTargetViews)
		{
			handle->ClearRenderTargetView(renderTargetView, pClearColor);
		}
		if (depthStencilView)
		{
			handle->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 0, 0);
		}

		ID3D11Texture2D *texture = colorAttachments[0];
		D3D11_TEXTURE2D_DESC desc{};
		texture->GetDesc(&desc);

		D3D11_VIEWPORT viewport = {
		    .TopLeftX = 0,
		    .TopLeftY = 0,
		    .Width    = FLOAT(desc.Width),
		    .Height   = FLOAT(desc.Height),
		    .MinDepth = 0.0f,
		    .MaxDepth = 1.0f,
		};
		handle->RSSetViewports(1, &viewport);
	});
}

void CommandBuffer::EndRenderTarget()
{

}

void CommandBuffer::GenerateMipMaps(SuperTexture *_texture, Filter filter)
{
	if (_texture->GetMipLevels() <= 1)
	{
		return;
	}

	Texture *texture = InterpretAs<Texture>(_texture);
	Submit([=, this] {
		handle->GenerateMips(texture->GetDescriptor());
	});
}

void CommandBuffer::CopyBufferToImage(SuperTexture *_texture, uint32_t subresource, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
{
	Buffer  *buffer  = InterpretAs<Buffer>(_buffer);
	Texture *texture = InterpretAs<Texture>(_texture);
	Submit([=, this] {
		uint8_t *data = nullptr;
		buffer->Map((void **)&data, buffer->GetSize(), 0);
		handle->UpdateSubresource(*texture, 0, nullptr, data + offset, bufferRowLength, 0);
		buffer->Unmap();
	});
}

void CommandBuffer::MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	Submit([=, this] {
		D3D11_BOX box = {
			.left   = offset,
			.top    = 0,
			.front  = 0,
			.right  = offset + size,
			.bottom = 1,
			.back   = 1,
		};
		handle->UpdateSubresource(*buffer, 0, &box, data, 0, 0);
	});
}

void CommandBuffer::MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	SLASSERT(false && "Don't call this function for Vulkan backend!");
}

void CommandBuffer::SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer)
{

}

void CommandBuffer::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	Submit([=, this] {
		handle->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	});
}

void CommandBuffer::DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	Submit([=, this] {
		handle->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	});
}

void CommandBuffer::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	Submit([=, this] {
		handle->Dispatch(nGroupX, nGroupY, nGroupZ);
	});
}

void CommandBuffer::DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{

}

void CommandBuffer::DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth)
{

}

}
}
