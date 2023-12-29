#include "RenderTarget.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{
namespace D3D11
{

RenderTarget::RenderTarget() :
    colorAttachments{},
    depthAttachment{}
{

}

RenderTarget::~RenderTarget()
{
	ClearAttachments();
}

void RenderTarget::Resize(UINT32 width, UINT32 height)
{
}

SuperTexture *RenderTarget::GetColorAttachment(uint32_t index)
{
	return &colorAttachments[index];
}

SuperTexture *RenderTarget::GetDepthAttachment()
{
	return depthAttachment ? & depthAttachment : nullptr;
}

void RenderTarget::AddColorAttachment(Texture &&texture)
{
	colorAttachments.clear();
	colorAttachments.emplace_back(std::move(texture));
}

void RenderTarget::ClearAttachments()
{
	depthAttachment = std::move(Texture{});
	colorAttachments.clear();
}

}
}
