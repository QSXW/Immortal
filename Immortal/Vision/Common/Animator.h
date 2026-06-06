/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

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

    double FPS() const
    {
        return FramesPerSecond;
    }

    double SPF() const
    {
        return SecondsPerFrame;
    }

    int64_t TotalSeconds() const
    {
        return Duration;
    }

    double TotalFrames() const
    {
        return TotalSeconds() * FPS();
    }

    double MoveToNextFrame()
    {
        return Accumulator = fmodf(Accumulator, SecondsPerFrame);
    }

    bool IsFrameDone() const
    {
        return Accumulator >= SecondsPerFrame;
    }

    bool TryMoveToNextFrame(float deltaTime)
    {
        Accumulator += deltaTime;
        if (IsFrameDone())
        {
            MoveToNextFrame();
            return true;
        }
        return false;
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
