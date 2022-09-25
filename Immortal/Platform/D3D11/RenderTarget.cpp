#include "RenderTarget.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

RenderTarget::RenderTarget()
{

}

RenderTarget::RenderTarget(Device *device, const RenderTarget::Description &descrition) :
    Super{ descrition },
    device{ device }
{
    for (size_t i = 0; i < descrition.Attachments.size(); i++)
    {
        auto &desc = descrition.Attachments[i];
        if (desc.format.IsDepth())
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = {
                .Format        = desc.format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D     = { .MipSlice = 0 }
            };

            depthAttachment.image = new Image{device, D3D11_BIND_DEPTH_STENCIL, desc.format, descrition.Width, descrition.Height, nullptr};
			Check(device->CreateDepthStencilView(*depthAttachment.image, &viewDesc, &depthAttachment.descriptor));
        }
        else
        {
            D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {
                .Format        = desc.format,
                .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
                .Texture2D     = { .MipSlice = 0 }
            };

            Attachment<RTV> attachment;
			attachment.image = new Image{device, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, desc.format, descrition.Width, descrition.Height, nullptr};
			Check(device->CreateRenderTargetView(*attachment.image, &viewDesc, &attachment.descriptor));
            colorAttachments.emplace_back(attachment);
			descriptors.emplace_back(attachment.descriptor);
        }
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {
        .Format        = colorAttachments[0].image->format,
        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        .Texture2D     = { .MostDetailedMip = 0, .MipLevels = 1, }
    };

    Check(device->CreateShaderResourceView(*colorAttachments[0].image, &viewDesc, &descriptor));
}

RenderTarget::RenderTarget(Device *device, Ref<Image> image)
{
	Attachment<RTV> attachment;
	D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {
	    .Format        = image->format,
	    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
	    .Texture2D     = { .MipSlice = 0 }
    };

    attachment.image = image;
	Check(device->CreateRenderTargetView(*attachment.image, &viewDesc, &attachment.descriptor));
	colorAttachments.emplace_back(attachment);
	descriptors.emplace_back(attachment.descriptor);
}

RenderTarget::~RenderTarget()
{

}

RenderTarget::operator uint64_t() const
{
	return descriptor;
}

void RenderTarget::Map(uint32_t slot)
{

}

void RenderTarget::Unmap()
{

}

void RenderTarget::Resize(UINT32 width, UINT32 height)
{

}

}
}
