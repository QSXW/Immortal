#pragma once

#include "Framework/Utils.h"
#include "Common.h"

namespace Immortal
{
namespace D3D11
{

using CommandList = ID3D11DeviceContext;

class Swapchain;
class Device
{
public: 
    using Primitive = ID3D11Device5;
    D3D11_OPERATOR_HANDLE()

public:
    Device(int deviceId);

    ~Device();

    HRESULT CreateFence(ID3D11Fence **ppFence, UINT64 initialValue, D3D11_FENCE_FLAG flags)
    {
        return handle->CreateFence(initialValue, flags, IID_PPV_ARGS(ppFence));
    }

    HRESULT CreateBuffer(const D3D11_BUFFER_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Buffer **ppBuffer)
    {
        return handle->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

    HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Texture2D **ppTexture2D)
    {
        return handle->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
    }

    HRESULT CreateShaderResourceView(ID3D11Resource *pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc, ID3D11ShaderResourceView **ppSRView)
    {
        return handle->CreateShaderResourceView(pResource, pDesc, ppSRView);
    }

    HRESULT CreateRenderTargetView(ID3D11Resource *pResource, const D3D11_RENDER_TARGET_VIEW_DESC *pDesc, ID3D11RenderTargetView **ppRTView)
    {
        return handle->CreateRenderTargetView(pResource, pDesc, ppRTView);
    }

    HRESULT CreateDepthStencilView(ID3D11Resource *pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc, ID3D11DepthStencilView **ppDepthStencilView)
    {
        return handle->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
    }

    HRESULT CreateUnorderedAccessView(ID3D11Resource *pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc, ID3D11UnorderedAccessView **ppUnorderedAccessView)
    {
        return handle->CreateUnorderedAccessView(pResource, pDesc, ppUnorderedAccessView);
    }

    HRESULT CreateRasterizerState(const D3D11_RASTERIZER_DESC *pRasterizerDesc, ID3D11RasterizerState **ppRasterizerState)
    {
        return handle->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
    }

    HRESULT CreateVertexShader(const void *pShaderBytecode, SIZE_T bytecodeLength, ID3D11ClassLinkage *pClassLinkage, ID3D11VertexShader **ppVertexShader)
    {
        return handle->CreateVertexShader(pShaderBytecode, bytecodeLength, pClassLinkage, ppVertexShader);
    }

    HRESULT CreatePixelShader(const void *pShaderBytecode, SIZE_T bytecodeLength, ID3D11ClassLinkage *pClassLinkage, ID3D11PixelShader **ppPixelShader)
    {
        return handle->CreatePixelShader(pShaderBytecode, bytecodeLength, pClassLinkage, ppPixelShader);
    }

    HRESULT CreateComputeShader(const void *pShaderBytecode,SIZE_T bytecodeLength, ID3D11ClassLinkage *pClassLinkage, ID3D11ComputeShader **ppComputeShader)
    {
        return handle->CreateComputeShader(pShaderBytecode, bytecodeLength, pClassLinkage, ppComputeShader);
    }

    HRESULT CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT numElements, const void *pShaderBytecodeWithInputSignature, SIZE_T bytecodeLength, ID3D11InputLayout **ppInputLayout)
    {
        return handle->CreateInputLayout(pInputElementDescs, numElements, pShaderBytecodeWithInputSignature, bytecodeLength, ppInputLayout);
    }

    HRESULT CreateSamplerState(const D3D11_SAMPLER_DESC *pSamplerDesc, ID3D11SamplerState **ppSamplerState)
    {
		return handle->CreateSamplerState(pSamplerDesc, ppSamplerState);
    }

    HRESULT Map(ID3D11Resource *pResource, UINT subresource, D3D11_MAP mapType, UINT mapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource)
    {
        return context->Map(pResource, subresource, mapType, mapFlags, pMappedResource);
    }

    void Map(ID3D11Resource *pResource, UINT subresource)
    {
        return context->Unmap(pResource, subresource);
    }

    void CopyResource(ID3D11Resource *pDstResource, ID3D11Resource *pSrcResource)
    {
        context->CopyResource(pDstResource, pSrcResource);
    }

    HRESULT CreateSwapchain(Swapchain *pSwapchain, DXGI_SWAP_CHAIN_DESC *pDesc);

public:
    template <class T = IDXGIFactory4>
    requires std::is_base_of_v<IDXGIFactory, T>
    T *GetDxgiFactory() const
    {
        return dxgiFactory.Get();
    }

    template <class T = ID3D11DeviceContext>
    requires std::is_base_of_v<ID3D11DeviceContext, T>
    T *GetContext() const
    {
        return context.Get();
    }

    template <class T = IDXGIAdapter1>
    requires std::is_base_of_v<IDXGIAdapter, T>
    T *GetAdapter() const
    {
        return adapter.Get();
    }

protected:
    ComPtr<IDXGIFactory4> dxgiFactory;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<ID3D11DeviceContext4> context;

    D3D_FEATURE_LEVEL featureLevel;
};

}
}
