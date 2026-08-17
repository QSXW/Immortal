#include "RenderTarget.h"
#include "Device.h"
#include "Texture.h"

#include <algorithm>

namespace Immortal
{
namespace D3D12
{

RenderTarget::RenderTarget(Device *device) :
    NonDispatchableHandle{ device },
    descriptors{},
    depthDescriptor{},
    renderTargetViewDescriptorHeap{},
    depthViewDescriptorHeap{},
    colorBuffers{},
    depth{}
{

}

RenderTarget::RenderTarget(Device *device, uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat, const ClearValue *pClearValues, uint32_t sampleCount) :
    RenderTarget{ device }
{
	colorBuffers.reserve(colorAttachmentCount);
    for (int i = 0; i < colorAttachmentCount; i++)
    {
	    Format format = pColorAttachmentFormats[i];
		const ClearValue *pColorClear = pClearValues ? &pClearValues[i] : nullptr;
		Ref<Texture> texture = new Texture{ device, format, width, height, 1/*(uint16_t)Texture::CalculateMipmapLevels(width, height)*/, 1, TextureType::ColorAttachment, sampleCount, pColorClear };
		SetColorAttachment(i, texture);
    }
	BuildRenderTargetView(sampleCount);

    if (depthAttachmentFormat != Format::None)
    {
		const ClearValue *pDepthClear = pClearValues ? &pClearValues[colorAttachmentCount] : nullptr;
		Ref<Texture> texture = new Texture{device, depthAttachmentFormat, width, height, 1/*(uint16_t) Texture::CalculateMipmapLevels(width, height)*/, 1, TextureType::DepthStencilAttachment, sampleCount, pDepthClear};
		SetDepthAttachment(texture);
    }
}
 
RenderTarget::~RenderTarget()
{
    if (descriptors)
    {
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetViewDescriptorHeap, descriptors, colorBuffers.size());
    }

    if (depthDescriptor)
    {
		device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthViewDescriptorHeap, depthDescriptor, 1);
    }
}

void RenderTarget::SetName(const char *s)
{
	std::string name = "RenderTarget::" + std::string(s);
    for (auto &c : colorBuffers)
    {
		c->SetName(name.c_str());
    }
    if (depth)
    {
		depth->SetName(name.c_str());
    }
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
}

void RenderTarget::BuildRenderTargetView(uint32_t sampleCount)
{
    if (colorBuffers.empty())
    {
		return;
    }

	descriptors = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, &renderTargetViewDescriptorHeap, colorBuffers.size());
    for (size_t i = 0;i  < colorBuffers.size(); i++)
    {
		auto &texture = colorBuffers[i];
		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {
		    .Format = texture->GetFormat(),
		    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
		    .Texture2D = {
		        .MipSlice   = 0,
		        .PlaneSlice = 0
            }
        };
		
        if (sampleCount > 1)
        {
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        }
		uint32_t arrayLayers = texture->GetArrayLayers();
		if (arrayLayers > 1)
		{
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray = {
			    .MipSlice        = 0,
			    .FirstArraySlice = 0,
			    .ArraySize       = arrayLayers,
			    .PlaneSlice      = 0
            };
		}

		device->CreateRenderTargetView(*texture, &viewDesc, descriptors[i]);
    }
}

void RenderTarget::SetDepthAttachment(Ref<Texture> &texture)
{
	depth = texture;
	depthDescriptor = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, &depthViewDescriptorHeap, 1);

	D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
	desc.Flags = D3D12_DSV_FLAG_NONE;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D     = { .MipSlice = 0 };
	device->CreateDepthStencilView(*texture, &desc, depthDescriptor);
}

}
}
