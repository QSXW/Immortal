#pragma once

#include "Core.h"
#include "Shared/Log.h"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#define METAL_SWAPPABLE(T) T(T &&other) : T{} { other.Swap(*this); } T &operator=(T &&other) { T(std::move(other)).Swap(*this); return *this; }  T(const T &) = delete; T &operator=(const T &) = delete;
#define METAL_OPERATOR_HANDLE() Primitive GetHandle() const { return handle; } operator Primitive() const { return handle; } protected: Primitive handle{}; public: operator bool() const { return !!handle; }

namespace Immortal
{
namespace Metal
{

enum class PushConstantIndexType
{
    Vertex   = 0,
    Fragment = 1,
    Compute  = 0,
};

constexpr uint32_t VertexBufferStartIndex = 30;

static inline void CheckError(NS::Error *error)
{
    if (error)
    {
        NS::StringEncoding encoding    = NS::StringEncoding::ASCIIStringEncoding;
        NS::String *description        = error->localizedDescription();
        NS::Array *recoveryOptions     = error->localizedRecoveryOptions();
        NS::String *recoverySuggestion = error->localizedRecoverySuggestion();
        NS::String *failureReason      = error->localizedFailureReason();
        if (description)
        {
            LOG::ERR("NS::Error - Description: {}", description->cString(encoding));
            description->release();
        }
        if (recoveryOptions)
        {
            for (uint32_t i = 0; i < recoveryOptions->count(); i++)
            {
                LOG::ERR("NS::Error - RecoveryOptions[{}]: {}", i, recoveryOptions->object<NS::String>(i)->cString(encoding));
            }
            recoveryOptions->release();
        }
        if (recoverySuggestion)
        {
            LOG::ERR("NS::Error - RecoverySuggestion: {}", recoverySuggestion->cString(encoding));
            recoverySuggestion->release();
        }
        if (failureReason)
        {
            LOG::ERR("NS::Error - FailureReason: {}", failureReason->cString(encoding));
            failureReason->release();
        }

        error->release();
    }
}

}
}
