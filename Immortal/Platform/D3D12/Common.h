#pragma once

#ifndef __D3D12_COMMON_H__
#define __D3D12_COMMON_H__

#include "ImmortalCore.h"

#ifdef WINDOWS

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <initguid.h>
#include <D3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3d12.lib")

#if SLDEBUG
#include <D3d12SDKLayers.h>
#endif

namespace Immortal
{
namespace D3D12
{

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

enum RootConstants
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

}
}

#endif
#endif // __D3D12_COMMON_H__
