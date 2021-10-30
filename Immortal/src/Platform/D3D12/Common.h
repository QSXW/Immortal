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

}
}

#endif
#endif // __D3D12_COMMON_H__
