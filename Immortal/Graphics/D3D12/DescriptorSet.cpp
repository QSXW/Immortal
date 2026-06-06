#include "DescriptorSet.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Buffer.h"
#include "BufferView.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace Immortal
{
namespace D3D12
{

static __forceinline void SetDescriptorSlot(DescriptorSet *descriptorSet, Descriptor baseDescriptor, uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	auto device = descriptorSet->GetDevice();
	device->CopyDescriptors(1, baseDescriptor[slot], descriptor, heapType);
}

DescriptorSet::DescriptorSet(Device *device, Pipeline *pipeline) :
	NonDispatchableHandle{ device },
    descriptorHeaps{},
    descriptors{},
    descriptorCount{},
    indexMap{}
{
	for (uint32_t i = 0; i < SL_ARRAY_LENGTH(descriptorHeaps); i++)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_CBV] = pipeline->GetDescriptorIndexMap(D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
			indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_SRV] = pipeline->GetDescriptorIndexMap(D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
			indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_UAV] = pipeline->GetDescriptorIndexMap(D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
		}
		else
		{
			indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER] = pipeline->GetDescriptorIndexMap(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
		}
		descriptorCount[i] = pipeline->GetDescriptorCount(type);
		if (descriptorCount[i] > 0)
		{
			device->AllocateShaderVisibleDescriptor(
				type,
				&descriptorHeaps[type],
				&descriptors[type],
			    descriptorCount[i]);
		}
	}
	rangeTypes = pipeline->GetDescriptorRangeType();
}

DescriptorSet::DescriptorSet(Device *device, uint32_t descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type) :
    NonDispatchableHandle{device},
    descriptorHeaps{},
    descriptors{},
    descriptorCount{descriptorCount},
    indexMap{}
{
	if (descriptorCount > 0)
	{
		device->AllocateShaderVisibleDescriptor(
		    type,
		    &descriptorHeaps[type],
		    &descriptors[type],
		    descriptorCount);
	}
}

DescriptorSet::~DescriptorSet()
{
	for (uint32_t i = 0; i < SL_ARRAY_LENGTH(descriptorHeaps); i++)
	{
		if (descriptorHeaps[i])
		{
			device->FreeShaderVisibleDescriptor((D3D12_DESCRIPTOR_HEAP_TYPE)i, descriptorHeaps[i], descriptors[i].descriptor, descriptorCount[i]);
		}
	}
}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *buffer)
{
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto &rangeType = rangeTypes[slot];
	auto descriptor = InterpretAs<Buffer>(buffer)->GetDescriptor();
	SetDescriptorSlot(this, descriptors[type], indexMap[rangeType][slot], D3D12_CPU_DESCRIPTOR_HANDLE(descriptor[rangeType]), type);
}

void DescriptorSet::Set(uint32_t slot, SuperBufferView *view)
{
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto &rangeType = rangeTypes[slot];
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = D3D12_CPU_DESCRIPTOR_HANDLE(InterpretAs<BufferView>(view)->GetDescriptor());
	SetDescriptorSlot(this, descriptors[type], indexMap[rangeType][slot], descriptorHandle, type);
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto &rangeType = rangeTypes[slot];
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = rangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV ? texture->GetUAVDescriptor(0) : texture->GetDescriptor();
	SetDescriptorSlot(this, descriptors[type], indexMap[rangeType][slot], descriptorHandle, type);
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *sampler)
{
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	SetDescriptorSlot(this, descriptors[type], indexMap[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER][slot], D3D12_CPU_DESCRIPTOR_HANDLE(InterpretAs<Sampler>(sampler)->GetDescriptor()), type);
}

void DescriptorSet::Set(uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_RANGE_TYPE rangeType)
{
	SetDescriptorSlot(this, descriptors[type], indexMap[rangeType][slot], descriptor, type);
}

}
}
