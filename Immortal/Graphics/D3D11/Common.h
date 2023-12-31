#pragma once

#ifndef __D3D11_COMMON_H__
#define __D3D11_COMMON_H__

#include "Core.h"

#include "D3D/Interface.h"
#include <d3d11_4.h>

namespace Immortal
{
namespace D3D11
{

using namespace D3D;
#define D3D11_OPERATOR_HANDLE() D3D_OPERATOR_PRIMITIVE(handle)
#define D3D11_SWAPPABLE(T) D3D_SWAPPABLE(T)

#define MAX_PUSH_CONSTANT_SIZE 128

using CommandList = ID3D11DeviceContext;

constexpr D3D_FEATURE_LEVEL FeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_11_1
};

struct PipelineStage
{
	enum
	{
		Vertex,
		Pixel,
		MaxTypes,
		Compute = Vertex,
	};
};

}
}

#endif
