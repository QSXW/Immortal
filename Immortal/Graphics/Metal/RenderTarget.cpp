#include "RenderTarget.h"
#include "Device.h"
#include "Metal/MTLTexture.hpp"
#include "Texture.h"

namespace Immortal
{
namespace Metal
{

RenderTarget::RenderTarget(Device *device)
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
	return &attachments.color[index];
}

SuperTexture *RenderTarget::GetDepthAttachment()
{
	return &attachments.depth;
}

void RenderTarget::SetColorAttachment(MTL::Texture *texture)
{
    attachments.color.clear();
    attachments.color.emplace_back(std::move(Texture{ texture }));
}

}
}
