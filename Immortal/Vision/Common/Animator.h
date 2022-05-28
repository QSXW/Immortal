#pragma once

#include "Core.h"
#include <cmath>

namespace Immortal
{

class Animator
{
public:
    Animator() :
        Timestamps{},
        Accumulator{},
        Timebase{},
        SecondsPerFrame{},
        FramesPerSecond{},
        Duration{},
        Step{}
    {
    }

public:
    struct {
        int64_t Last;
        int64_t Current;
    } Timestamps;

    double Accumulator;
    double Timebase;
    double SecondsPerFrame;
    double FramesPerSecond;
    int64_t Duration;
    uint32_t Step;
};

}
