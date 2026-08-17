#pragma once

#include "Graphics/DescriptorSet.h"
#include "Descriptor.h"
#include "Handle.h"

#include <vector>

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline;
class DescriptorHeap;
class Texture;
class IMMORTAL_API DescriptorSet : public SuperDescriptorSet, public NonDispatchableHandle
{
public:
	static constexpr uint32_t MaxDescrpitorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1;

	struct TextureBinding
	{
		uint32_t slot;
		Texture *texture;
		D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
	};

public:
	DescriptorSet(Device *device, Pipeline *pipeline);

	DescriptorSet(Device *device, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type);

    virtual ~DescriptorSet() override;

	virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperBufferView *view) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	void SetUavMip(uint32_t slot, SuperTexture *texture, uint32_t mipSlice) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

	void Set(uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_RANGE_TYPE rangeType);

public:
	DescriptorHeap *GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
		return descriptorHeaps[type];
    }

	ShaderVisibleDescriptor GetDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return descriptors[type];
	}

	const std::vector<TextureBinding> &GetTextureBindings() const
	{
		return textureBindings;
	}

protected:
	void TrackTextureBinding(uint32_t slot, Texture *texture, D3D12_DESCRIPTOR_RANGE_TYPE rangeType);

	void ClearTextureBinding(uint32_t slot);

protected:
	Pipeline *pipeline;

	DescriptorHeap *descriptorHeaps[MaxDescrpitorHeapType];

    ShaderVisibleDescriptor descriptors[MaxDescrpitorHeapType];

	const uint32_t *indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER + 1];

	uint32_t descriptorCount[MaxDescrpitorHeapType];

	const D3D12_DESCRIPTOR_RANGE_TYPE *rangeTypes;

	std::vector<TextureBinding> textureBindings;
};

}
}
