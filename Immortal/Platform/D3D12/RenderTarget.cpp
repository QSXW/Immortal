#include "impch.h"
#include "RenderTarget.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

void PixelBuffer::Create(const Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue)
{
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask     = 1;
    heapProperties.VisibleNodeMask      = 1;

    device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        &resource
    );
}

void ColorBuffer::Create(const Device *device, const Description &desc, const D3D12_CLEAR_VALUE &clearValue)
{
    Super::Create(device, desc, clearValue);
}

RenderTarget::RenderTarget()
{

}

RenderTarget::RenderTarget(const Device *device, const RenderTarget::Description &descrition) :
    Super{ descrition }
{
    D3D12_CLEAR_VALUE clearValue{};
    for (auto &attachment : descrition.Attachments)
    {
        D3D12_RESOURCE_FLAGS flags = attachment.IsDepth() ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL:
                                                            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                                            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        Resource::Description resourceDesc = SuperToBase(
            descrition,
            attachment.BaseFromat<DXGI_FORMAT>(),
            flags
            );
        clearValue.Format = resourceDesc.Format;
        if (attachment.IsDepth())
        {
            clearValue.DepthStencil.Depth   = Super::clearValues.depthStencil.depth;
            clearValue.DepthStencil.Stencil = Super::clearValues.depthStencil.stencil;
            attachments.depth.Create(device, resourceDesc, clearValue);
        }
        else
        {
            clearValue.Color[0] = Super::clearValues.color.r;
            clearValue.Color[1] = Super::clearValues.color.g;
            clearValue.Color[2] = Super::clearValues.color.b;
            clearValue.Color[3] = Super::clearValues.color.a;

            attachments.color.emplace_back(ColorBuffer{});
            auto &colorBuffer = attachments.color.back();
            colorBuffer.Create(device, resourceDesc, clearValue);
        }
    }

#ifdef SLDEBUG
    attachments.depth.Set(L"RenderTarget::DepthStencilAttachment");
    for (size_t i = 0; i < attachments.color.size(); i++)
    {
        attachments.color[i].Set(std::wstring{ L"RenderTarget::DepthStencilAttachment#" } + std::to_wstring(i));
    }
#endif
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
