#include "impch.h"
#include "RenderTarget.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

RenderTarget::RenderTarget()
{

}

RenderTarget::RenderTarget(const Device *device, const RenderTarget::Description &descrition) :
    Super{ descrition }
{
    for (auto &attachment : descrition.Attachments)
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (attachment.IsDepth())
        {
            flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        D3D12_RESOURCE_DESC resourceDesc = SuperToBase(
            descrition,
            attachment.BaseFromat<DXGI_FORMAT>(),
            flags
        );
        int pause = 0;
    }
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
