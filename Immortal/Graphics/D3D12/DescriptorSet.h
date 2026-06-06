#pragma once

#include "Graphics/DescriptorSet.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline;
class DescriptorHeap;
class IMMORTAL_API DescriptorSet : public SuperDescriptorSet, public NonDispatchableHandle
{
public:
	static constexpr uint32_t MaxDescrpitorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1;

public:
	DescriptorSet(Device *device, Pipeline *pipeline);

    virtual ~DescriptorSet() override;

	virtual void Set(uint32_t slot, SuperBuffer *buffer) override;

	virtual void Set(uint32_t slot, SuperTexture *texture) override;

	virtual void Set(uint32_t slot, SuperSampler *sampler) override;

	void Set(uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE type);

public:
	DescriptorHeap *GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
		return descriptorHeaps[type];
    }

	ShaderVisibleDescriptor GetDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return descriptors[type];
	}

protected:
	DescriptorHeap *descriptorHeaps[MaxDescrpitorHeapType];

    ShaderVisibleDescriptor descriptors[MaxDescrpitorHeapType];

	const uint32_t *indexMap[MaxDescrpitorHeapType];

	uint32_t descriptorCount[MaxDescrpitorHeapType];
};

}
}
