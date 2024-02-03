#include "DescriptorSet.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace Immortal
{
namespace Metal
{

template <class T, class S, class M>
inline void SetTemplate(uint32_t slot, S *s, std::vector<M> &data)
{
	T *t = InterpretAs<T>(s);
	if (slot >= data.size())
	{
		data.resize(slot + 1);
	}
    data[slot] = *t;
}

template <class T, class M>
void SetResource(MTL::RenderCommandEncoder *renderCommandEncoder, const std::vector<M> &data)
{
	for (size_t i = 0, j = 0; i < data.size(); i++)
	{
		if (data[i])
		{
			if constexpr (std::is_same_v<T, Texture>)
			{
				renderCommandEncoder->setFragmentTexture(data[i], j);
			}
			else if constexpr (std::is_same_v<T, Sampler>)
			{
				renderCommandEncoder->setFragmentSamplerState(data[i], j);
			}
			else if constexpr (std::is_same_v<T, Buffer>)
			{
				renderCommandEncoder->setVertexBuffer(data[i], 0, j);
				renderCommandEncoder->setFragmentBuffer(data[i], 0, j);
			}
            j++;
		}
	}
}

template <class T, class M>
void SetResource(MTL::ComputeCommandEncoder *computeCommandEncoder, const std::vector<M> &data)
{
	for (size_t i = 0, j = 0; i < data.size(); i++)
	{
		if (data[i])
		{
			if constexpr (std::is_same_v<T, Texture>)
			{
				computeCommandEncoder->setTexture(data[i], j);
			}
			else if constexpr (std::is_same_v<T, Sampler>)
			{
				computeCommandEncoder->setSamplerState(data[i], j);
			}
			else if constexpr (std::is_same_v<T, Buffer>)
			{
				computeCommandEncoder->setBuffer(data[i], 0, j);
			}
            j++;
		}
	}
}

DescriptorSet::DescriptorSet(Device *device, Pipeline *pipeline) :
    buffers{},
    textures{},
    samplers{}
{

}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *_buffer)
{
	SetTemplate<Buffer>(slot, _buffer, buffers);
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	SetTemplate<Texture>(slot, _texture, textures);
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *_sampler)
{
	SetTemplate<Sampler>(slot, _sampler, samplers);
}

void DescriptorSet::Set(MTL::RenderCommandEncoder *renderCommandEncoder)
{
	SetResource<Buffer>(renderCommandEncoder, buffers);
	SetResource<Texture>(renderCommandEncoder, textures);
	SetResource<Sampler>(renderCommandEncoder, samplers);
}

void DescriptorSet::Set(MTL::ComputeCommandEncoder *computeEncodeEncoder)
{
	SetResource<Buffer>(computeEncodeEncoder,  buffers);
	SetResource<Texture>(computeEncodeEncoder, textures);
	SetResource<Sampler>(computeEncodeEncoder, samplers);
}

}
}
