#pragma once

#ifndef __D3D11_COMMON_H__
#define __D3D11_COMMON_H__

#include "Core.h"

#include "Platform/D3D/Interface.h"
#include <d3d11_4.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d11.lib")

namespace Immortal
{
namespace D3D11
{

using namespace D3D;
#define D3D11_OPERATOR_HANDLE() D3D_OPERATOR_PRIMITIVE(handle)

constexpr D3D_FEATURE_LEVEL FeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_11_1
};

static inline void GetHardwareAdapter(IDXGIFactory4 *pFactory, IDXGIAdapter1 **ppAdapter)
{
	*ppAdapter = nullptr;
	for (UINT adapterIndex = 0;; adapterIndex++)
	{
		IDXGIAdapter1 *pAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
		{
			break;
		}

		if (SUCCEEDED(D3D11CreateDevice(
			pAdapter,
		    D3D_DRIVER_TYPE_UNKNOWN,
			0,
			0,
			FeatureLevels,
		    SL_ARRAY_LENGTH(FeatureLevels),
		    D3D11_SDK_VERSION,
			nullptr,
			nullptr,
			nullptr
			)))
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
