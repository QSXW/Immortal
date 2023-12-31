#include "DescriptorSet.h"
#include "Common.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace Immortal
{
namespace D3D11
{

DescriptorSet::DescriptorSet(Pipeline *pipeline)
{

}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *_buffer)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	bufferDescriptorTable[slot] = *buffer;
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	shaderResourceViewTable[slot] = texture->GetDescriptor();
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *_sampler)
{
	Sampler *sampler = InterpretAs<Sampler>(_sampler);
	samplerDescriptorTable[slot] = *sampler;
}

}
}
