#pragma once

#ifndef NETWORK_ABSTRACTION_LAYER_H__
#define NETWORK_ABSTRACTION_LAYER_H__

#include "Core.h"

namespace Immortal
{

struct IMMORTAL_API NetworkAbstractionLayer
{
    enum Type : int32_t
    {
        Corrupted = -1,
        TRAIL_N = 0,
        TRAIL_R,
        VPP = 32,
        SPS,
        PPS,
        AUD,
        EOS,
        EOB,
        FD,
        PREFIX_SEI,
        SUFFIX_SEI,
        None = ~0
    };

    virtual int32_t GetType() const
    {
        return Type::None;
    }

    struct Header {};
    struct Payload {};
};

using NAL = NetworkAbstractionLayer;
using SuperNetworkAbstractionLayer = NAL;

}

#endif // !NETWORK_ABSTRACTION_LAYER_H__
