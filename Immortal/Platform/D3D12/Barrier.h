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
            *this = Transition(std::forward<Args>(args)...);
        }
        if constexpr (T == BarrierType::Aliasing)
        {
            *this = Aliasing(std::forward<Args>(args)...);
        }
        if constexpr (T == BarrierType::UAV)
        {
            *this = UAV(std::forward<Args>(args)...);
        }
    }

    static inline Barrier Transition(
        ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
    {
        Barrier result{};
        Primitive &barrier = result;

        result.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        result.Flags                   = flags;
        barrier.Transition.pResource   = pResource;
        barrier.Transition.StateBefore = stateBefore;
        barrier.Transition.StateAfter  = stateAfter;
        barrier.Transition.Subresource = subresource;

        return result;
    }

    static inline Barrier Aliasing(ID3D12Resource* pResourceBefore, ID3D12Resource* pResourceAfter) noexcept
    {
        Barrier result{};
        Primitive &barrier = result;

        result.Type                      = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barrier.Aliasing.pResourceBefore = pResourceBefore;
        barrier.Aliasing.pResourceAfter  = pResourceAfter;

        return result;
    }

    static inline Barrier UAV(ID3D12Resource* pResource) noexcept
    {
        Barrier result{};
        D3D12_RESOURCE_BARRIER &barrier = result;

        result.Type           = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = pResource;

        return result;
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
