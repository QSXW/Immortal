#pragma once

#include "ImmortalCore.h"

#ifdef _MSC_VER

#include <D3d12.h>
#pragma comment(lib, "D3d12.dll")

#if SLDEBUG
#include <D3d12SDKLayers.h>
#pragma comment(lib, "D3d12SDKLayers.dll")
#endif

#endif
