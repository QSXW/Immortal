#include "DescriptorSet.h"
#include "Common.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace Immortal
{
namespace OpenGL
{

DescriptorSet::DescriptorSet(Pipeline *pipeline) :
    descriptors{ pipeline->GetDescriptors() },
    sampler{}
{

}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *_buffer)
{
	if (descriptors.size() >= slot)
	{
		descriptors.resize(slot + 1);
	}

	Buffer *buffer = InterpretAs<Buffer>(_buffer);
	descriptors[slot].handle  = *buffer;
	descriptors[slot].binding = slot;
	descriptors[slot].type    = DescriptorType::UniformBuffer;
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	if (descriptors.size() >= slot)
	{
		descriptors.resize(slot + 1);
	}

    Texture *texture = InterpretAs<Texture>(_texture);
	descriptors[slot].handle  = *texture;
	descriptors[slot].binding = slot;
	descriptors[slot].format  = texture->GetFormat();
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *_sampler)
{
	sampler = *InterpretAs<Sampler>(_sampler);
}

void DescriptorSet::BindDescriptorTable()
{
	for (size_t i = 0;  i < descriptors.size(); i++)
	{
		auto &desciptor = descriptors[i];
		if (!desciptor.handle)
		{
			continue;
		}
		switch (desciptor.type)
		{
			case DescriptorType::UniformBuffer:
				glBindBufferBase(GL_UNIFORM_BUFFER, desciptor.binding, desciptor.handle);
				break;
			case DescriptorType::Sampler:
				glBindSampler(desciptor.binding, desciptor.handle);
			case DescriptorType::SamplerImage2D:
				glBindTextureUnit(desciptor.binding, desciptor.handle);
				if (sampler)
				{
					glBindSampler(desciptor.binding, sampler);
				}
				break;
			case DescriptorType::Image2D:
				glBindImageTexture(desciptor.binding, desciptor.handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, desciptor.format);
				if (sampler)
				{
					glBindSampler(desciptor.binding, sampler);
				}
				break;
			default:
				break;
		}		
	}
}

}
}
