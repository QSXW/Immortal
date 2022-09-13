#pragma once

#ifndef __D3D12_COMMON_H__
#define __D3D12_COMMON_H__

#include "Core.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <initguid.h>
#include <D3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3d12.lib")

#if _DEBUG
#include <D3d12SDKLayers.h>
#endif

namespace Immortal
{
namespace D3D12
{

#define D3D12_OPERATOR_PRITIMIVE(name)  Primitive *Handle() const { return name.Get(); } operator Primitive*() const { return Handle(); } protected: ComPtr<Primitive> name;
#define D3D12_OPERATOR_HANDLE() D3D12_OPERATOR_PRITIMIVE(handle)

namespace Definitions
{

constexpr uint32_t RootConstantsIndex  = 8;
constexpr uint32_t ConstantBufferIndex = 0;

}

namespace Limit
{

constexpr uint32_t BytesOfRootConstant = 128;

}

template <void(*F), class ... A>
void IfNotNullThen(A&& ... args)
{
    F(std::forward<A>(args)...);
}

template <class T>
void IfNotNullThenRelease(T ptr)
{
    if (ptr)
    {
        ptr->Release();
    }
}

enum
{
    ReferenceWhiteNits = 0,
    DisplayCurve,
    RootConstantsCount
};

enum DisplayCurve
{
    sRGB = 0,
    ST2084,
    None
};

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL    ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

static const float HDRMetaDataPool[4][4] = {
    // MaxOutputNits, MinOutputNits,  MaxCLL, MaxFALL
    {        1000.0f,        0.001f, 2000.0f,  500.0f },
    {         500.0f,        0.001f, 2000.0f,  500.0f },
    {         500.0f,        0.100f,  500.0f,  100.0f },
    {        2000.0f,        1.000f, 2000.0f, 1000.0f }
};

struct DisplayChromaticities
{
    float RedX;
    float RedY;
    float GreenX;
    float GreenY;
    float BlueX;
    float BlueY;
    float WhiteX;
    float WhiteY;
};

static inline void Check(HRESULT result, const char *message = "")
{
    if (FAILED(result))
    {
        LOG::ERR("Status Code => {0}", GetLastError());
        if (!message || !message[0])
        {
            LOG::ERR("{0}", "This is a DirectX 12 Execption. Check Output for more details...");
        }
        else
        {
            LOG::ERR("{0}", message);
        }
        throw Exception(message);
    }
}

static inline void GetHardwareAdapter(IDXGIFactory4 *pFactory, IDXGIAdapter1 **ppAdapter)
{
    *ppAdapter = nullptr;
    for (UINT adapterIndex = 0; ; adapterIndex++)
    {
        IDXGIAdapter1 *pAdapter = nullptr;
        if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
        {
            // No more adapters to enumerate.
            break;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
        {
            *ppAdapter = pAdapter;
            return;
        }
        pAdapter->Release();
    }
}

/*
 * @brief Concise Helper Structures
 */

#define TYPE_HINT(S) \
    "Only "#S" Type is acceptable"

#define ASSERT_TYPES4(is_type, T, hint) \
    static_assert(is_type<T##0>() && is_type<T##1>() && is_type<T##2>() && is_type<T##3>() && hint);

struct Viewport : public D3D12_VIEWPORT
{
    using Primitive = D3D12_VIEWPORT;

    Viewport()
    {

    }

    explicit Viewport(const Primitive &p) noexcept :
        Primitive{ p }
    {

    }

    template <class T0, class T1, class T2, class T3>
    explicit Viewport(
        T0 topLeftX,
        T1 topLeftY,
        T2 width,
        T3 height,
        float minDepth = D3D12_MIN_DEPTH,
        float maxDepth = D3D12_MAX_DEPTH) noexcept
    {
        ASSERT_TYPES4(std::is_arithmetic, T, TYPE_HINT(Arithmetic))

        TopLeftX = FLOAT(topLeftX);
        TopLeftY = FLOAT(topLeftY);
        Width    = FLOAT(width);
        Height   = FLOAT(height);
        MinDepth = FLOAT(minDepth);
        MaxDepth = FLOAT(maxDepth);
    }
};

struct Rect : public D3D12_RECT
{
    using Primitive = D3D12_RECT;

    Rect()
    {

    }

    explicit Rect(const Primitive &p) noexcept :
        Primitive{ p }
    {

    }

    template <class T0, class T1, class T2, class T3>
    explicit Rect(T0 left, T1 top, T2 right, T3 bottom) noexcept
    {
        ASSERT_TYPES4(std::is_arithmetic, T, TYPE_HINT(Arithmetic))
        Primitive::left   = LONG(left);
        Primitive::top    = LONG(top);
        Primitive::right  = LONG(right);
        Primitive::bottom = LONG(bottom);
    }
};

struct DescriptorRange : public D3D12_DESCRIPTOR_RANGE1
{
    using Primitive = D3D12_DESCRIPTOR_RANGE1;

    DescriptorRange()
    {

    }

    explicit DescriptorRange(const Primitive &p) noexcept :
        Primitive{ p }
    {

    }

    DescriptorRange(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
    {
        Init(rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
    }

    inline void Init(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        UINT offsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
    {
        Init(*this, rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
    }

    static inline void Init(
        Primitive &range,
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
        UINT offsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept
    {
        range.RangeType                         = rangeType;
        range.NumDescriptors                    = numDescriptors;
        range.BaseShaderRegister                = baseShaderRegister;
        range.RegisterSpace                     = registerSpace;
        range.Flags                             = flags;
        range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};

struct RasterizerDescription : public D3D12_RASTERIZER_DESC
{
    using Primitive = D3D12_RASTERIZER_DESC;

    RasterizerDescription()
    {
        Primitive::FillMode = D3D12_FILL_MODE_SOLID;
        Primitive::CullMode = D3D12_CULL_MODE_NONE;
        Primitive::FrontCounterClockwise = FALSE;
        Primitive::DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        Primitive::DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        Primitive::SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        Primitive::DepthClipEnable = TRUE;
        Primitive::MultisampleEnable = FALSE;
        Primitive::AntialiasedLineEnable = FALSE;
        Primitive::ForcedSampleCount = 0;
        Primitive::ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }

    explicit RasterizerDescription(const Primitive &o) noexcept :
        Primitive{ o }
    {

    }

    explicit RasterizerDescription(
        D3D12_FILL_MODE fillMode,
        D3D12_CULL_MODE cullMode,
        BOOL frontCounterClockwise,
        INT depthBias,
        FLOAT depthBiasClamp,
        FLOAT slopeScaledDepthBias,
        BOOL depthClipEnable,
        BOOL multisampleEnable,
        BOOL antialiasedLineEnable,
        UINT forcedSampleCount,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster) noexcept
    {
        Primitive::FillMode = fillMode;
        Primitive::CullMode = cullMode;
        Primitive::FrontCounterClockwise = frontCounterClockwise;
        Primitive::DepthBias = depthBias;
        Primitive::DepthBiasClamp = depthBiasClamp;
        Primitive::SlopeScaledDepthBias = slopeScaledDepthBias;
        Primitive::DepthClipEnable = depthClipEnable;
        Primitive::MultisampleEnable = multisampleEnable;
        Primitive::AntialiasedLineEnable = antialiasedLineEnable;
        Primitive::ForcedSampleCount = forcedSampleCount;
        Primitive::ConservativeRaster = conservativeRaster;
    }
};

struct BlendDescription : public D3D12_BLEND_DESC
{
    using Primitive = D3D12_BLEND_DESC;

    explicit BlendDescription() noexcept
    {
        Primitive::AlphaToCoverageEnable = FALSE;
        Primitive::IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDescription =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            Primitive::RenderTarget[i] = defaultRenderTargetBlendDescription;
        }
    }

    explicit BlendDescription(const D3D12_BLEND_DESC &o) noexcept :
        Primitive{ o }
    {

    }
};

struct DepthStencilDescription : public D3D12_DEPTH_STENCIL_DESC
{
    using Primitive = D3D12_DEPTH_STENCIL_DESC;

    static inline const D3D12_DEPTH_STENCILOP_DESC DefaultStencilOp = {
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_COMPARISON_FUNC_ALWAYS
    };

    explicit DepthStencilDescription() noexcept
    {
        Primitive::DepthEnable      = TRUE;
        Primitive::DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
        Primitive::DepthFunc        = D3D12_COMPARISON_FUNC_LESS;
        Primitive::StencilEnable    = FALSE;
        Primitive::StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        Primitive::StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        Primitive::FrontFace        = DefaultStencilOp;
        Primitive::BackFace         = DefaultStencilOp;
    }
};

}
}

#endif // __D3D12_COMMON_H__
