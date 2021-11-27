#include "impch.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace D3D12
{

RenderTarget::RenderTarget()
{

}

RenderTarget::RenderTarget(const RenderTarget::Description &descrition)
{

}

RenderTarget::~RenderTarget()
{

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

void *RenderTarget::ReadPixel(UINT32 attachmentIndex, int x, int y, Format format, int width, int height)
{
    return nullptr;
}

void RenderTarget::ClearAttachment(UINT32 attachmentIndex, int value)
{

}

}
}
