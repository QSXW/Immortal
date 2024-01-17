#include "CommandBuffer.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Texture.h"
#include "Pipeline.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace OpenGL
{

CommandBuffer::CommandBuffer(const std::string &name) :
    name{name},
    viewport{}
{

}

void CommandBuffer::Execute()
{
	while (!recorder.empty())
	{
		auto &command = recorder.front();
		command();
		recorder.pop();
	}
}

CommandBuffer::~CommandBuffer()
{

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
		glUseProgram(*pipeline);
		pipeline->SetState();
	});
}

void CommandBuffer::SetDescriptorSet(SuperDescriptorSet *_descriptorSet)
{
	DescriptorSet *descriptorSet = InterpretAs<DescriptorSet>(_descriptorSet);
	Submit([=] {
		descriptorSet->BindDescriptorTable();
	});
}

void CommandBuffer::SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **_ppBuffer, uint32_t /*strideInBytes*/)
{
	if (bufferCount > 1)
	{
		LOG::ERR("OpenGL only support bind one vertex buffer!");
	}
	Buffer **ppBuffer = (Buffer **)_ppBuffer;
	Buffer *buffer = ppBuffer[0];
	Submit([=, this] {
		auto vertexArray = pipeline->GetVextexArray();
		vertexArray->Activate(buffer);
		vertexArray->Bind();
		glBindBuffer(GL_ARRAY_BUFFER, *buffer);
	});
}

void CommandBuffer::SetIndexBuffer(SuperBuffer *_buffer, Format format)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	Submit([=, this] {
		indexFormat = format;
		indexSize   = format.Size();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	});
}

void CommandBuffer::SetScissors(uint32_t count, const Rect *pScissor)
{
	auto width  =  (int)pScissor[0].right - pScissor[0].left;
	auto height =  (int)pScissor[0].bottom - pScissor[0].top;
	auto x      =  (int)pScissor[0].left;
	auto y      =  (int)pScissor[0].bottom;
	Submit([=, this] {
		glScissor(x, viewport.height - y, width, height);
	});
}

void CommandBuffer::SetBlendFactor(const float factor[4])
{

}

void CommandBuffer::PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset)
{
	if (!pushConstant || pushConstant->GetSize() < size)
	{
		pushConstant = new Buffer{ size, BufferType::ConstantBuffer };
	}
	pushConstant->Update(size, pData, offset);
	Submit([=, this] {
		glBindBufferBase(GL_UNIFORM_BUFFER, PUSH_CONSTANT_LOCATION, *pushConstant);
	});
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const float *pClearColor)
{
	float red   = pClearColor[0];
	float green = pClearColor[1];
	float blue  = pClearColor[2];
	float alpha = pClearColor[3];

	RenderTarget *renderTarget = InterpretAs<RenderTarget>(_renderTarget);

	Submit([=, this] {
		glBindFramebuffer(GL_FRAMEBUFFER, *renderTarget);
		viewport.width  = renderTarget->GetWidth();
		viewport.height = renderTarget->GetHeight();
		glViewport(0, 0, viewport.width, viewport.height);
		glClearColor(red, green, blue, alpha);
		glClear(GL_COLOR_BUFFER_BIT);
	});
}

void CommandBuffer::EndRenderTarget()
{
	Submit([=, this] {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}

void CommandBuffer::GenerateMipMaps(SuperTexture *_texture, Filter filter)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	Submit([=, this] {
		glGenerateTextureMipmap(*texture);
	});
}

void CommandBuffer::CopyBufferToImage(SuperTexture *_texture, uint32_t subresource, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
{
	Buffer  *buffer  = InterpretAs<Buffer>(_buffer);
	Texture *texture = InterpretAs<Texture>(_texture);
	Submit([=] {
		glPixelStorei(GL_PACK_ROW_LENGTH, bufferRowLength);
		glTextureSubImage2D(*texture, 0, 0, 0, texture->GetWidth(), texture->GetHeight(), texture->GetBaseFormat(), texture->GetBinaryFormat(), buffer->GetMemory() + offset);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	});
}

void CommandBuffer::MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	Submit([=] {
		buffer->Bind();
		glBufferSubData(buffer->GetBindPoint(), offset, size, data);
		buffer->Unbind();
	});
}

void CommandBuffer::MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch)
{

}

void CommandBuffer::SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer)
{

}

void CommandBuffer::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{

}

void CommandBuffer::DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	Submit([=, this] {
		for (uint32_t i = 0; i < instanceCount; i++)
		{
			uint64_t indexLocation = (startIndexLocation + i * indexCountPerInstance) * indexSize;
			glDrawElementsBaseVertex(GL_TRIANGLES, indexCountPerInstance, indexFormat, (void *)indexLocation, baseVertexLocation);
		}
	});
}

void CommandBuffer::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	Submit([=] {
		glDispatchCompute(nGroupX, nGroupY, nGroupZ);
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
