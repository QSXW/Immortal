#include "RenderTarget.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{
namespace D3D12
{

RenderTarget::RenderTarget(Device *device) :
    NonDispatchableHandle{ device }
{

}

RenderTarget::~RenderTarget()
{

}

void RenderTarget::Resize(UINT32 width, UINT32 height)
{

}

SuperTexture *RenderTarget::GetColorAttachment(uint32_t index)
{
	return colorBuffers[index].Get();
}

SuperTexture *RenderTarget::GetDepthAttachment()
{
	return depth.Get();
}

void RenderTarget::SetColorAttachment(uint32_t index, Ref<Texture> &texture)
{
    if (index >= colorBuffers.size())
    {
		colorBuffers.resize(index);
    }

	colorBuffers.emplace_back(texture);
    descriptors[index] = texture->GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {
		.Format        = texture->GetFormat(),
	    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
        .Texture2D = {
            .MipSlice   = 0,
            .PlaneSlice = 0
        }
    };

    uint32_t arrayLayers = texture->GetArrayLayers();
	if (arrayLayers > 1)
	{
		viewDesc.ViewDimension  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray = {
			.MipSlice        = 0,
			.FirstArraySlice = 0,
			.ArraySize       = arrayLayers,
            .PlaneSlice      = 0
        }; 
	}

    device->CreateRenderTargetView(*texture, &viewDesc, descriptors[index]);
}

void RenderTarget::SetDepthAttachment(Ref<Texture> &texture)
{
	depth = texture;
	D3D12_DEPTH_STENCIL_VIEW_DESC desc{
	    .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
        .Texture2D = {
            .MipSlice = 0
        }
    };

    uint32_t arrayLayers = texture->GetArrayLayers();
    if (arrayLayers > 1)
    {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
        desc.Texture2DArray = {
            .MipSlice        = 0,
            .FirstArraySlice = 0,
			.ArraySize       = arrayLayers
        };
    }

    depthDescriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(*texture, &desc, depthDescriptor);
}

}
}
