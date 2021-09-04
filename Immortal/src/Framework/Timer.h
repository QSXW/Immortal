#pragma once

#include <chrono>

namespace Immortal
{
class Timer
{
public:
    using Seconds      = std::ratio<1>;
    using Milliseconds = std::ratio<1, 1000>;
    using Microseconds = std::ratio<1, 1000000>;
    using Nanoseconds  = std::ratio<1, 1000000000>;

    using Clock             = std::chrono::steady_clock;
    using DefaultResolution = Milliseconds;

public:
    Timer() : start{ Clock::now() }, previousTick{ start } { }
        
    virtual ~Timer() = default;

    void Start()
    {
        if (!running)
        {
            running = true;
            start = Clock::now();
        }
    }
        
    void Lap()
    {
        lapping = true;
        lap = Clock::now();
    }

    bool Running() const
    {
        return running;
    }

    template <class T = DefaultResolution>
    double Stop()
    {
        if (!running)
        {
            return 0;
        }

        running = false;
        lapping  = false;

        auto now = Clock::now();
        auto duration = std::chrono::duration<double, T>(now - start);
        start = now;
        lap = now;

        return duration.count();
    }
        
    template <class T = DefaultResolution>
    double elapsed()
    {
        if (!running)
        {
            return 0;
        }

        if (lapping)
        {
            start = lap;
        }

        return std::chrono::duration<double, T>(Clock::now() - start).count();
    }

    template <class T = DefaultResolution>
    double tick()
    {
        auto now = Clock::now();
        auto duration = std::chrono::duration<double, T>(now - previousTick);
        previousTick = now;
        return duration();
    }

private:
    bool running{ false };
    bool lapping{ false };

    Clock::time_point start;
    Clock::time_point lap;
    Clock::time_point previousTick;
};
}
