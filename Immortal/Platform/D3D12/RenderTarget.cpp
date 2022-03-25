#include "impch.h"
#include "RenderTarget.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

void PixelBuffer::Create(const Device *device, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE &clearValue)
{
    Format = desc.Format;

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

void ColorBuffer::Create(Device *device, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE &clearValue)
{
    Super::Create(device, desc, clearValue);

    struct {
        D3D12_RENDER_TARGET_VIEW_DESC RenderTarget;
        D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResource;
        D3D12_UNORDERED_ACCESS_VIEW_DESC UnorderedAccess;
    } viewDescription{};
    
    viewDescription.RenderTarget.Format    = Format;
    viewDescription.ShaderResource.Format  = Format;
    viewDescription.UnorderedAccess.Format = GetUAVFormat(Format);

    viewDescription.ShaderResource.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (desc.DepthOrArraySize > 1)
    {
        viewDescription.RenderTarget.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        viewDescription.RenderTarget.Texture2DArray.MipSlice        = 0;
        viewDescription.RenderTarget.Texture2DArray.FirstArraySlice = 0;
        viewDescription.RenderTarget.Texture2DArray.ArraySize       = (UINT)desc.DepthOrArraySize;

        viewDescription.UnorderedAccess.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        viewDescription.UnorderedAccess.Texture2DArray.MipSlice        = 0;
        viewDescription.UnorderedAccess.Texture2DArray.FirstArraySlice = 0;
        viewDescription.UnorderedAccess.Texture2DArray.ArraySize       = (UINT)desc.DepthOrArraySize;

        viewDescription.ShaderResource.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDescription.ShaderResource.Texture2DArray.MipLevels       = desc.MipLevels;
        viewDescription.ShaderResource.Texture2DArray.MostDetailedMip = 0;
        viewDescription.ShaderResource.Texture2DArray.FirstArraySlice = 0;
        viewDescription.ShaderResource.Texture2DArray.ArraySize       = (UINT)desc.DepthOrArraySize;
    }
    else if (desc.SampleDesc.Count > 1)
    {
        viewDescription.RenderTarget.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        viewDescription.ShaderResource.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    }
    else
    {
        viewDescription.RenderTarget.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE2D;
        viewDescription.RenderTarget.Texture2D.MipSlice = 0;

        viewDescription.UnorderedAccess.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
        viewDescription.UnorderedAccess.Texture2D.MipSlice = 0;

        viewDescription.ShaderResource.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDescription.ShaderResource.Texture2D.MipLevels       = desc.MipLevels;
        viewDescription.ShaderResource.Texture2D.MostDetailedMip = 0;
    }

    if (renderTargetViewDescriptor.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        renderTargetViewDescriptor   = RenderContext::AllocateDescriptor(DescriptorPool::Type::RenderTargetView);
        shaderResourceViewDescriptor = RenderContext::AllocateShaderVisibleDescriptor();
    }
   
    device->CreateView(resource, &viewDescription.RenderTarget, renderTargetViewDescriptor);
    device->CreateView(resource, &viewDescription.ShaderResource, shaderResourceViewDescriptor.cpu);

    if (desc.SampleDesc.Count > 1)
    {
        return;
    }

    for (uint32_t i = 0; i < desc.MipLevels; ++i)
    {
        if (unorderedAccessViewDescriptor[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        {
            unorderedAccessViewDescriptor[i] = RenderContext::AllocateDescriptor(DescriptorPool::Type::ShaderResourceView);
        }
        device->CreateView(resource, nullptr, &viewDescription.UnorderedAccess, unorderedAccessViewDescriptor[i]);
        viewDescription.UnorderedAccess.Texture2D.MipSlice++;
    }
}

void DepthBuffer::Create(Device *device, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE &clearValue)
{
    Super::Create(device, desc, clearValue);

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription{};

    depthStencilViewDescription.Format = GetDSVFormat(desc.Format);
    if (desc.SampleDesc.Count == 1)
    {
        depthStencilViewDescription.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDescription.Texture2D.MipSlice = 0;
    }
    else
    {
        depthStencilViewDescription.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    }

    if (depthStencilViewDescriptor[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        depthStencilViewDescriptor[0] = RenderContext::AllocateDescriptor(DescriptorPool::Type::DepthStencilView);
        depthStencilViewDescriptor[1] = RenderContext::AllocateDescriptor(DescriptorPool::Type::DepthStencilView);
    }

    depthStencilViewDescription.Flags = D3D12_DSV_FLAG_NONE;
    device->CreateView(resource, &depthStencilViewDescription, depthStencilViewDescriptor[0]);

    depthStencilViewDescription.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    device->CreateView(resource, &depthStencilViewDescription, depthStencilViewDescriptor[1]);

    DXGI_FORMAT stencilReadFormat = GetStencilFormat(desc.Format);
    if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
    {
        if (depthStencilViewDescriptor[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        {
            depthStencilViewDescriptor[2] = RenderContext::AllocateDescriptor(DescriptorPool::Type::DepthStencilView);
            depthStencilViewDescriptor[3] = RenderContext::AllocateDescriptor(DescriptorPool::Type::DepthStencilView);
        }

        depthStencilViewDescription.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
        device->CreateView(resource, &depthStencilViewDescription, depthStencilViewDescriptor[2]);

        depthStencilViewDescription.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
        device->CreateView(resource, &depthStencilViewDescription, depthStencilViewDescriptor[3]);
    }
    else
    {
        depthStencilViewDescriptor[2] = depthStencilViewDescriptor[0];
        depthStencilViewDescriptor[3] = depthStencilViewDescriptor[1];
    }

    if (shaderResourceDescriptor.depth.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
    {
        shaderResourceDescriptor.depth = RenderContext::AllocateDescriptor(DescriptorPool::Type::ShaderResourceView);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc{};
    shaderResourceDesc.Format = GetDepthFormat(desc.Format);
    if (depthStencilViewDescription.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
    {
        shaderResourceDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
        shaderResourceDesc.Texture2D.MipLevels = 1;
    }
    else
    {
        shaderResourceDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    }

    shaderResourceDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    device->CreateView(resource, &shaderResourceDesc, shaderResourceDescriptor.depth);

    if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
    {
        if (shaderResourceDescriptor.stencil.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        {
            shaderResourceDescriptor.stencil = RenderContext::AllocateDescriptor(DescriptorPool::Type::ShaderResourceView);
        }

        shaderResourceDesc.Format = stencilReadFormat;
        device->CreateView(resource, &shaderResourceDesc, shaderResourceDescriptor.stencil);
    }
}

RenderTarget::RenderTarget()
{

}

RenderTarget::RenderTarget(Device *device, const RenderTarget::Description &descrition) :
    Super{ descrition }
{
    D3D12_CLEAR_VALUE clearValue{};
    size_t index = 0;
    for (auto &attachment : descrition.Attachments)
    {
        D3D12_RESOURCE_FLAGS flags = attachment.IsDepth() ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL :
                                                            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                                            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        D3D12_RESOURCE_DESC resourceDesc = SuperToBase(
            descrition,
            attachment.format,
            flags
            );
        clearValue.Format = resourceDesc.Format;
        if (attachment.IsDepth())
        {
            clearValue.DepthStencil.Depth   = 1.0f;
            clearValue.DepthStencil.Stencil = 0.0f;
            attachments.depth.Create(device, resourceDesc, clearValue);
        }
        else
        {
            auto &clearValues = Super::clearValues[index];
            clearValue.Color[0] = clearValues.r;
            clearValue.Color[1] = clearValues.g;
            clearValue.Color[2] = clearValues.b;
            clearValue.Color[3] = clearValues.a;

            ColorBuffer colorBuffer{};
            colorBuffer.Create(device, resourceDesc, clearValue);
            attachments.color.emplace_back(std::move(colorBuffer));
            descriptors[index] = colorBuffer.Get<Descriptor::Type::RenderTargetView>();
            index++;
        }
    }
    
#ifdef SLDEBUG
    attachments.depth.SetName(L"RenderTarget::DepthStencilAttachment");
    for (size_t i = 0; i < attachments.color.size(); i++)
    {
        attachments.color[i].SetName(std::wstring{ L"RenderTarget::ColorAttachment#" } + std::to_wstring(i));
    }
#endif
}

RenderTarget::~RenderTarget()
{
    attachments.color.clear();
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

void RenderTarget::ClearAttachment(UINT32 attachmentIndex, int value)
{

}

}
}
