#pragma once

#include "ImmortalCore.h"

#ifdef WINDOWS

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <D3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3d12.lib")

#if SLDEBUG
#define D3DCOMPILE_DEBUG  1
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
    if (!ptr)
    {
        ptr->Release();
    }
}

static inline void Check(HRESULT result, const char *message = "")
{
    if (FAILED(result))
    {
        LOG::ERR("Status Code => {0}", result);
        if (!message || !message[0])
        {
            LOG::ERR("{0}", "Unknow Exception");
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
