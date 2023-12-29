#pragma once

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

enum class BarrierType
{
    Transition,
    Aliasing,
    UAV
};

constexpr auto Transition = BarrierType::Transition;
constexpr auto UAV        = BarrierType::UAV;

template <BarrierType T>
struct Barrier : public D3D12_RESOURCE_BARRIER
{
    using Primitive = D3D12_RESOURCE_BARRIER;

    Barrier() = default;

    explicit Barrier(const Primitive & o) noexcept :
        Primitive{ o }
    {
    
    }

    template <class ... Args>
    Barrier(Args&&... args)
    {
        if constexpr (T == BarrierType::Transition)
        {
            Transition(std::forward<Args>(args)...);
        }
        if constexpr (T == BarrierType::Aliasing)
        {
            Aliasing(std::forward<Args>(args)...);
        }
        if constexpr (T == BarrierType::UAV)
        {
            UAV(std::forward<Args>(args)...);
        }
    }

    void Transition(
        ID3D12Resource *pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
    {
        Primitive::Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Primitive::Flags                  = flags;
        Primitive::Transition.pResource   = pResource;
        Primitive::Transition.StateBefore = stateBefore;
        Primitive::Transition.StateAfter  = stateAfter;
        Primitive::Transition.Subresource = subresource;
    }

    void Aliasing(ID3D12Resource *pResourceBefore, ID3D12Resource *pResourceAfter) noexcept
    {
        Primitive::Type                     = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        Primitive::Aliasing.pResourceBefore = pResourceBefore;
        Primitive::Aliasing.pResourceAfter  = pResourceAfter;
    }

    void UAV(ID3D12Resource *pResource) noexcept
    {
        Primitive::Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        Primitive::UAV.pResource = pResource;
        Primitive::Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    }

    void Swap()
    {
        if constexpr (T == BarrierType::Transition)
        {
            std::swap(Primitive::Transition.StateBefore, Primitive::Transition.StateAfter);
        }
    }

    operator D3D12_RESOURCE_BARRIER&()
    {
        return *this;
    }
};

}
}
