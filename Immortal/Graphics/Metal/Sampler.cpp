#include "Sampler.h"
#include "Device.h"
#include "Metal/MTLDepthStencil.hpp"
#include "Metal/MTLSampler.hpp"
#include "Types.h"

namespace Immortal
{
namespace Metal
{

Sampler::Sampler() :
	Handle<MTL::SamplerState>{}
{

}

Sampler::Sampler(Device *device, Filter filter, AddressMode _addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
    Handle<MTL::SamplerState>{}
{
	MTL::SamplerDescriptor *descriptor = MTL::SamplerDescriptor::alloc()->init();
	descriptor->setBorderColor(MTL::SamplerBorderColorTransparentBlack);
	descriptor->setCompareFunction(MTL::CompareFunction(compareOperation));
	descriptor->setLodMinClamp(minLod);
	descriptor->setLodMaxClamp(maxLod);

	MTL::SamplerAddressMode addressMode = MTL::SamplerAddressModeClampToEdge;
	if (_addressMode == AddressMode::Mirror)
	{
		addressMode = MTL::SamplerAddressModeMirrorRepeat;
	}
	else if (_addressMode == AddressMode::BorderColor)
	{
		addressMode = MTL::SamplerAddressModeClampToBorderColor;
	}
	descriptor->setTAddressMode(addressMode);
	descriptor->setSAddressMode(addressMode);
	descriptor->setRAddressMode(addressMode);

	MTL::SamplerMipFilter    mipFilter    = MTL::SamplerMipFilterNearest;
	MTL::SamplerMinMagFilter minMagFilter = MTL::SamplerMinMagFilterNearest;
	if (filter != Filter::Nearest)
	{
		mipFilter    = MTL::SamplerMipFilterLinear;
		minMagFilter = MTL::SamplerMinMagFilterLinear;
		if (filter != Filter::Anisotropic)
		{
			descriptor->setMaxAnisotropy(16);
		}
	}
	descriptor->setMagFilter(minMagFilter);
	descriptor->setMinFilter(minMagFilter);
	descriptor->setMipFilter(mipFilter);

	handle = device->GetHandle()->newSamplerState(descriptor);
    descriptor->release();
}

Sampler::~Sampler()
{

}

}
}
