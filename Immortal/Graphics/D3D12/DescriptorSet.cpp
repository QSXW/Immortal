#include "DescriptorSet.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Pipeline.h"

namespace Immortal
{
namespace D3D12
{

static __forceinline void SetDescriptorSlot(DescriptorSet *descriptorSet, DescriptorHeap *descriptorHeap, Descriptor baseDescriptor, uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	auto device = descriptorSet->GetDevice();
	device->CopyDescriptors(1, baseDescriptor[slot], descriptor, heapType);
}

DescriptorSet::DescriptorSet(Device *device, Pipeline *pipeline) :
	NonDispatchableHandle{ device },
    descriptorHeaps{},
    descriptors{}
{
	auto shaderResourceType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	device->AllocateShaderVisibleDescriptor(
	    shaderResourceType,
	    &descriptorHeaps[shaderResourceType],
	    &descriptors[shaderResourceType],
	    pipeline->GetDescriptorCount(shaderResourceType));
	indexMap[shaderResourceType] = pipeline->GetDescriptorIndexMap(shaderResourceType);

	auto samplerType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	device->AllocateShaderVisibleDescriptor(
	    samplerType,
	    &descriptorHeaps[samplerType],
	    &descriptors[samplerType],
	    pipeline->GetDescriptorCount(samplerType));

	indexMap[samplerType] = pipeline->GetDescriptorIndexMap(samplerType);
}

DescriptorSet::~DescriptorSet()
{

}

void DescriptorSet::Set(uint32_t slot, SuperBuffer *buffer)
{
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SetDescriptorSlot(this, descriptorHeaps[type], descriptors[type], indexMap[type][slot], D3D12_CPU_DESCRIPTOR_HANDLE(InterpretAs<Buffer>(buffer)->GetDescriptor()), type);
}

void DescriptorSet::Set(uint32_t slot, SuperTexture *_texture)
{
	Texture *texture = InterpretAs<Texture>(_texture);
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SetDescriptorSlot(this, descriptorHeaps[type], descriptors[type], indexMap[type][slot], D3D12_CPU_DESCRIPTOR_HANDLE(texture->GetDescriptor()), type);
}

void DescriptorSet::Set(uint32_t slot, SuperSampler *sampler)
{
	constexpr auto type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	SetDescriptorSlot(this, descriptorHeaps[type], descriptors[type], indexMap[type][slot], D3D12_CPU_DESCRIPTOR_HANDLE(InterpretAs<Sampler>(sampler)->GetDescriptor()), type);
}

void DescriptorSet::Set(uint32_t slot, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	SetDescriptorSlot(this, descriptorHeaps[type], descriptors[type], indexMap[type][slot], descriptor, type);
}

}
}
