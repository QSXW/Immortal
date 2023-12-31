#pragma once

#include "Graphics/DescriptorSet.h"
#include "Graphics/LightGraphics.h"
#include "Descriptor.h"
#include "Common.h"

namespace Immortal
{
namespace D3D11
{

class Pipeline;
class IMMORTAL_API DescriptorSet : public SuperDescriptorSet
{
public:
	DescriptorSet(Pipeline *pipeline);

	virtual ~DescriptorSet() override;

	virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

public:
	const std::unordered_map<uint32_t, ID3D11ShaderResourceView *> &GetShaderResourceViewTable() const
	{
		return shaderResourceViewTable;
	}

	const std::unordered_map<uint32_t, ID3D11SamplerState *> &GetSamplerDescriptorTable() const
	{
		return samplerDescriptorTable;
	}

	const std::unordered_map<uint32_t, ID3D11Buffer *> &GetBufferDescriptorTable() const
	{
		return bufferDescriptorTable;
	}

protected:
	std::unordered_map<uint32_t, ID3D11ShaderResourceView *> shaderResourceViewTable;

	std::unordered_map<uint32_t, ID3D11SamplerState *> samplerDescriptorTable;

	std::unordered_map<uint32_t, ID3D11Buffer *> bufferDescriptorTable;
};

}
}
