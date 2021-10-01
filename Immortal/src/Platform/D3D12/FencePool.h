#pragma once

#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{

class Fence
{
public:
    enum class Type
    {
        None               = D3D12_FENCE_FLAG_NONE,
        Shared             = D3D12_FENCE_FLAG_SHARED,
        SharedCrossAdapter = D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
        NonMonitored       = D3D12_FENCE_FLAG_NON_MONITORED
    };
};

}
}
